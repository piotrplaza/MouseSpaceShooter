#include "gameHelpers.hpp"

#include <systems/deferredActions.hpp>

#include <components/plane.hpp>
#include <components/physics.hpp>
#include <components/decoration.hpp>
#include <components/missile.hpp>
#include <components/shockwave.hpp>
#include <components/mvp.hpp>
#include <components/camera2D.hpp>
#include <components/screenInfo.hpp>
#include <components/animatedTexture.hpp>
#include <components/deferredAction.hpp>
#include <components/sound.hpp>
#include <components/audioListener.hpp>

#include <commonTypes/componentMappers.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>
#include <globals/collisionBits.hpp>

#include <ogl/shaders/textured.hpp>
#include <ogl/shaders/julia.hpp>
#include <ogl/shaders/particles.hpp>

#include <tools/Shapes2D.hpp>

#include <set>

namespace Tools
{
	MissileHandler::MissileHandler() = default;

	MissileHandler::MissileHandler(ComponentId missileId, ComponentId backThrustId, glm::vec2 referenceVelocity, std::optional<ComponentId> planeId,
		std::optional<ComponentId> soundId):
		missileId(missileId),
		backThrustId(backThrustId),
		referenceVelocity(referenceVelocity),
		planeId(planeId),
		soundId(soundId)
	{
	}

	MissileHandler::~MissileHandler()
	{
		if (!valid) return;

		Globals::Components().missiles()[missileId].state = ComponentState::Outdated;
		Globals::Components().dynamicDecorations()[backThrustId].state = ComponentState::Outdated;

		if (soundId)
			Globals::Components().sounds()[*soundId].state = ComponentState::Outdated;
	}

	MissileHandler::MissileHandler(MissileHandler&& other) noexcept:
		missileId(other.missileId),
		backThrustId(other.backThrustId),
		referenceVelocity(other.referenceVelocity),
		planeId(other.planeId),
		soundId(other.soundId)
	{
		other.valid = false;
	}

	MissileHandler& MissileHandler::operator=(MissileHandler&& other) noexcept
	{
		missileId = other.missileId;
		backThrustId = other.backThrustId;
		referenceVelocity = other.referenceVelocity;
		planeId = other.planeId;
		soundId = other.soundId;
		other.valid = false;
		return *this;
	}

	SoundsLimitter::SoundsLimitter(unsigned limit) :
		limit(limit)
	{
	}

	std::shared_ptr<SoundsLimitter> SoundsLimitter::create(unsigned limit) {
		return std::shared_ptr<SoundsLimitter>(new SoundsLimitter(limit));
	}

	void SoundsLimitter::newSound(Components::Sound& sound)
	{
		if (sounds.size() == limit)
			sounds.front()->immediateFreeResources();

		sounds.push_back(&sound);

		sound.tearDownF = [this, soundsLimitter = shared_from_this(), it = std::prev(sounds.end())]() {
			sounds.erase(it);
		};
	}

	const Tools::BodyParams& GetDefaultParamsForPlaneBody()
	{
		static const auto planeBodyDefaultParams = Tools::BodyParams().bodyType(b2_dynamicBody).density(0.2f).restitution(0.1f).autoSleeping(false).bullet(true).linearDamping(0.1f).angularDamping(15.0);
		return planeBodyDefaultParams;
	}

	ComponentId CreatePlane(Body body, ComponentId planeTexture, ComponentId thrustAnimatedTexture, PlaneParams params)
	{
		auto& plane = Globals::Components().planes().emplace(std::move(body), CM::StaticTexture(planeTexture));

		plane.setOrigin(params.position_);
		plane.setAngle(params.angle_);
		plane.posInSubsequence = params.numOfThrusts_ + params.collisionBoxRendering_;

		if (params.collisionBoxRendering_)
		{
			plane.subsequence.emplace_back(plane.getVertices());
			plane.subsequence.back().modelMatrixF = plane.modelMatrixF;
		}

		for (int i = 0; i < params.numOfThrusts_; ++i)
		{
			auto& planeDecoration = plane.subsequence.emplace_back(Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 0.5f, 0.5f }), std::vector<glm::vec2>{},
				CM::StaticAnimatedTexture(thrustAnimatedTexture));

			planeDecoration.renderingSetupF = [&, i,
				modelUniform = UniformsUtils::UniformMat4f(),
				thrust = 1.0f,
				thrustAnimatedTexture,
				params
			](ShadersUtils::ProgramId program) mutable {
					if (!modelUniform.isValid()) modelUniform = UniformsUtils::UniformMat4f(program, "model");
					modelUniform(
						glm::translate(
							glm::scale(
								glm::rotate(
									glm::translate(Tools::GetModelMatrix(*plane.body), { params.thrustOffset_.x, (i == 0 ? -1 : 1) * params.thrustOffset_.y, 0.0f }),
									-glm::half_pi<float>() + (i == 0 ? 1 : -1) * params.thrustAngle_, { 0.0f, 0.0f, 1.0f }),
								{ 0.5f + thrust * 0.02f, thrust, 1.0f }),
							{ 0.0f, -0.5f, 0.0f }));

					const float targetThrust = 1.0f + plane.details.throttleForce * 0.3f;
					const float changeStep = Globals::Components().physics().frameDuration * 10.0f;

					if (thrust < targetThrust)
						thrust = std::min(thrust + changeStep, targetThrust);
					else
						thrust = std::max(thrust - changeStep, targetThrust);

					Globals::Components().staticAnimatedTextures()[thrustAnimatedTexture].setSpeedScaling(1.0f + (thrust - 1) * 0.2f);

					glBlendFunc(GL_SRC_ALPHA, GL_ONE);

					return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
				};
		}

		return plane.getComponentId();
	}

	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 referenceVelocity,
		glm::vec2 initialVelocity, ComponentId missileTexture, ComponentId thrustAnimatedTexture, std::optional<ComponentId> planeId,
		std::optional<ComponentId> missileSoundBuffer)
	{
		auto& missile = Globals::Components().missiles().emplace(Tools::CreateBoxBody({ 0.5f, 0.2f },
			Tools::BodyParams().position(startPosition).angle(startAngle).bodyType(b2_dynamicBody).density(0.2f)), force);

		auto& body = *missile.body;
		SetCollisionFilteringBits(body, Globals::CollisionBits::missile, Globals::CollisionBits::all - Globals::CollisionBits::missile - Globals::CollisionBits::plane);
		body.SetLinearVelocity(ToVec2<b2Vec2>(referenceVelocity + initialVelocity));
		body.SetBullet(true);

		missile.texture = CM::StaticTexture(missileTexture);

		missile.renderingSetupF = [
			modelUniform = UniformsUtils::UniformMat4f(), &body](ShadersUtils::ProgramId program) mutable
			{
				if (!modelUniform.isValid()) modelUniform = UniformsUtils::UniformMat4f(program, "model");
				modelUniform(Tools::GetModelMatrix(body));
				return nullptr;
			};

		missile.renderLayer = RenderLayer::FarMidground;

		auto& decoration = Globals::Components().dynamicDecorations().emplace(Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, -0.5f }, { 0.5f, 0.5f }),
			CM::StaticAnimatedTexture(thrustAnimatedTexture), Tools::Shapes2D::CreateTexCoordOfRectangle());

		decoration.renderingSetupF = [&, modelUniform = UniformsUtils::UniformMat4f(),
			thrustScale = 0.1f
		](ShadersUtils::ProgramId program) mutable {
				if (!modelUniform.isValid()) modelUniform = UniformsUtils::UniformMat4f(program, "model");
				modelUniform(glm::scale(glm::rotate(glm::translate(Tools::GetModelMatrix(*missile.body),
					{ -0.5f, 0.0f, 0.0f }),
					-glm::half_pi<float>(), { 0.0f, 0.0f, 1.0f }),
					{ std::min(thrustScale * 0.2f, 0.3f), thrustScale, 1.0f }));

				const float targetFrameDurationFactor = Globals::Components().physics().frameDuration * 6.0f;
				thrustScale = std::min(thrustScale * (1.0f + targetFrameDurationFactor), 3.0f);

				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
			};

		decoration.renderLayer = RenderLayer::FarMidground;

		std::optional<ComponentId> soundId;
		if (missileSoundBuffer)
		{
			soundId = Tools::CreateAndPlaySound(*missileSoundBuffer,
				[&]() {
					return missile.getOrigin2D();
				},
				[](auto& sound) {
					sound.setRemoveOnStop(false);
				}).getComponentId();
		}

		return { missile.getComponentId(), decoration.getComponentId(), referenceVelocity, planeId, soundId };
	}

	void CreateExplosion(ExplosionParams params)
	{
		auto& particlesShaders = Globals::Shaders().particles();

		Globals::Components().deferredActions().emplace([=, &particlesShaders](float) {
			auto& shockwave = Globals::Components().shockwaves().emplace(params.center_, params.sourceVelocity_, params.numOfParticles_, params.initExplosionVelocity_,
				params.initExplosionVelocityRandomMinFactor_, params.particlesRadius_, params.particlesDensity_, params.particlesLinearDamping_, params.particlesAsBullets_);
			auto& explosionDecoration = Globals::Components().dynamicDecorations().emplace();
			explosionDecoration.customShadersProgram = particlesShaders.getProgramId();
			explosionDecoration.resolutionMode = params.resolutionMode_;
			explosionDecoration.drawMode = GL_POINTS;
			explosionDecoration.bufferDataUsage = GL_DYNAMIC_DRAW;

			explosionDecoration.renderingSetupF = [=, startTime = Globals::Components().physics().simulationDuration,
				&particlesShaders](ShadersUtils::ProgramId program) mutable
				{
					particlesShaders.vp(Globals::Components().mvp2D().getVP());
					particlesShaders.texture1(params.explosionTexture_);

					const float elapsed = Globals::Components().physics().simulationDuration - startTime;
					particlesShaders.color(glm::vec4(glm::vec3(glm::pow(1.0f - elapsed / (params.explosionDuration_ * 2.0f), 10.0f)), 1.0f));

					glBlendFunc(GL_SRC_ALPHA, GL_ONE);

					return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
				};

			explosionDecoration.renderLayer = RenderLayer::FarForeground;

			Globals::Components().deferredActions().emplace([=, startTime = Globals::Components().physics().simulationDuration, &shockwave, &explosionDecoration](float) {
				const float elapsed = Globals::Components().physics().simulationDuration - startTime;
				const float scale = 1.0f + elapsed * 20.0f;

				if (elapsed > params.explosionDuration_)
				{
					shockwave.state = ComponentState::Outdated;
					explosionDecoration.state = ComponentState::Outdated;
					return false;
				}

				explosionDecoration.vertices.clear();
				for (size_t i = 0; i < shockwave.particles.size(); ++i)
				{
					if (i % params.particlesPerDecoration_ != 0) continue;
					const auto& particle = shockwave.particles[i];
					const glm::vec2 position = shockwave.center + (ToVec2<glm::vec2>(particle->GetWorldCenter()) - shockwave.center) * 0.5f;
					explosionDecoration.vertices.emplace_back(position, scale);
				}
				explosionDecoration.state = ComponentState::Changed;

				return true;
			});

			return false;
			});
	}

	void CreateFogForeground(int numOfLayers, float alphaPerLayer, ComponentId fogTexture, FVec4 fColor, std::function<glm::vec2(int layer)> textureTranslation)
	{
		for (int layer = 0; layer < numOfLayers; ++layer)
		for (int posYI = -1; posYI <= 1; ++posYI)
		for (int posXI = -1; posXI <= 1; ++posXI)
		{
			auto renderingSetupF = [=, texturedProgram = ShadersUtils::Programs::TexturedAccessor()
			](ShadersUtils::ProgramId program) mutable {
				if (!texturedProgram.isValid()) texturedProgram = program;
				texturedProgram.vp(glm::translate(glm::scale(Globals::Components().mvp2D().getVP(), glm::vec3(glm::vec2(100.0f), 0.0f)),
					glm::vec3(-Globals::Components().camera2D().details.prevPosition * (0.002f + layer * 0.002f), 0.0f)));
				texturedProgram.color(fColor() * glm::vec4(1.0f, 1.0f, 1.0f, alphaPerLayer));

				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				return [texturedProgram]() mutable {
					texturedProgram.vp(Globals::Components().mvp2D().getVP());
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				};
			};

			auto& fogLayer = Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreateVerticesOfRectangle({ posXI, posYI }, glm::vec2(2.0f, 2.0f) + (layer * 0.2f)),
				CM::DummyTexture(), Tools::Shapes2D::CreateTexCoordOfRectangle(), std::move(renderingSetupF));
			fogLayer.renderLayer = RenderLayer::Foreground;
			fogLayer.stepF = [=, &fogLayer]() {
				fogLayer.texture = CM::StaticTexture(fogTexture, textureTranslation ? textureTranslation(layer) : glm::vec2(0.0f));
			};
			fogLayer.resolutionMode = { ResolutionMode::Resolution::H540, ResolutionMode::Scaling::Linear };
		}
	}

	void CreateJuliaBackground(JuliaParams params)
	{
		auto& juliaShaders = Globals::Shaders().julia();
		auto& background = Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 10.0f, 10.0f }));
		background.customShadersProgram = juliaShaders.getProgramId();

		background.renderingSetupF = [=, &juliaShaders, &screenInfo = Globals::Components().screenInfo()](auto) {
			juliaShaders.vp(glm::translate(glm::scale(glm::mat4(1.0f),
				glm::vec3(1.0f / screenInfo.getAspectRatio(), 1.0f, 1.0f) * 1.5f),
				glm::vec3(-Globals::Components().camera2D().details.prevPosition * 0.005f, 0.0f)));
			juliaShaders.juliaC(params.juliaCF_());
			juliaShaders.juliaCOffset(params.juliaCOffsetF_());
			juliaShaders.minColor(params.minColorF_());
			juliaShaders.maxColor(params.maxColorF_());
			juliaShaders.iterations(params.iterationsF_());
			return nullptr;
		};

		background.renderLayer = RenderLayer::Background;
		background.resolutionMode = { ResolutionMode::Resolution::Native, ResolutionMode::Scaling::Linear, ResolutionMode::Blending::Additive };
	}

	Components::Sound& CreateAndPlaySound(ComponentId soundBuffer, FVec2 posF, std::function<void(Components::Sound&)> config, std::function<void(Components::Sound&)> stepF)
	{
		auto& sound = Globals::Components().sounds().emplace(soundBuffer);

		sound.setRemoveOnStop(true);
		sound.stepF = [&, posF = std::move(posF), stepF = std::move(stepF)]()
		{
			if (posF.isLoaded())
				sound.setPosition(posF());

			if (stepF)
				stepF(sound);
		};

		if (config)
			config(sound);

		sound.play();

		return sound;
	}

	std::function<void(b2Fixture&, b2Fixture&)> SkipDuplicatedBodiesCollisions(std::function<void(b2Fixture&, b2Fixture&)> handler)
	{
		return [prevSimulationDuration = -1.0f, prevCollisions = std::set<std::pair<const b2Body*, const b2Body*>>(), handler](b2Fixture& fixture1, b2Fixture& fixture2) mutable
		{
			const float simulationDuration = Globals::Components().physics().simulationDuration;
			const auto bodies = Sort(fixture1.GetBody(), fixture2.GetBody());

			if (prevSimulationDuration != simulationDuration)
			{
				prevCollisions.clear();
				prevSimulationDuration = simulationDuration;
			}

			if (!prevCollisions.contains(bodies))
			{
				handler(fixture1, fixture2);
				prevCollisions.insert(bodies);
			}
		};
	}
}
