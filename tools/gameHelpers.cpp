#include "gameHelpers.hpp"

#include <systems/deferredActions.hpp>

#include <components/plane.hpp>
#include <components/physics.hpp>
#include <components/decoration.hpp>
#include <components/missile.hpp>
#include <components/shockwave.hpp>
#include <components/renderingSetup.hpp>
#include <components/mvp.hpp>
#include <components/camera.hpp>
#include <components/screenInfo.hpp>
#include <components/animatedTexture.hpp>
#include <components/deferredAction.hpp>
#include <components/sound.hpp>

#include <commonTypes/typeComponentMappers.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>
#include <globals/collisionBits.hpp>

#include <ogl/shaders/textured.hpp>
#include <ogl/shaders/julia.hpp>
#include <ogl/shaders/particles.hpp>

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

	ComponentId CreatePlane(ComponentId planeTexture, ComponentId flameAnimatedTexture, glm::vec2 position, float angle)
	{
		auto& plane = Globals::Components().planes().emplace(Tools::CreatePlaneBody(2.0f, 0.2f, 0.5f), TCM::Texture(planeTexture));

		auto& body = *plane.body;
		body.SetBullet(true);

		plane.setPosition(position);
		plane.setRotation(angle);
		plane.preserveTextureRatio = true;
		plane.posInSubsequence = 2;

		for (int i = 0; i < 2; ++i)
		{
			auto& planeDecoration = plane.subsequence.emplace_back(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 0.5f, 0.5f }), std::vector<glm::vec2>{},
				TCM::AnimatedTexture(flameAnimatedTexture));

			Globals::Components().renderingSetups().emplace([&, i,
				modelUniform = Uniforms::UniformMat4f(),
				thrust = 1.0f,
				flameAnimatedTexture
			](Shaders::ProgramId program) mutable {
					if (!modelUniform.isValid()) modelUniform = Uniforms::UniformMat4f(program, "model");
					modelUniform(
						glm::translate(
							glm::scale(
								glm::rotate(
									glm::translate(Tools::GetModelMatrix(*plane.body), { -0.5f, i == 0 ? -0.4f : 0.4f, 0.0f }),
									-glm::half_pi<float>() + (i == 0 ? 0.1f : -0.1f), { 0.0f, 0.0f, 1.0f }),
								{ 0.5f + thrust * 0.02f, thrust, 1.0f }),
							{ 0.0f, -0.5f, 0.0f }));

					const float targetThrust = 1.0f + plane.details.throttleForce * 0.3f;
					const float changeStep = Globals::Components().physics().frameDuration * 10.0f;

					if (thrust < targetThrust)
						thrust = std::min(thrust + changeStep, targetThrust);
					else
						thrust = std::max(thrust - changeStep, targetThrust);

					Globals::Components().animatedTextures()[flameAnimatedTexture].setSpeedScaling(1.0f + (thrust - 1) * 0.2f);

					glBlendFunc(GL_SRC_ALPHA, GL_ONE);

					return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
				});

			planeDecoration.renderingSetup = Globals::Components().renderingSetups().size() - 1;
		}

		return plane.getComponentId();
	}

	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 referenceVelocity,
		glm::vec2 initialVelocity, ComponentId missileTexture, ComponentId flameAnimatedTexture, std::optional<ComponentId> planeId,
		std::optional<ComponentId> missileSoundBuffer)
	{
		auto& missile = Globals::Components().missiles().emplace(Tools::CreateBoxBody({ 0.5f, 0.2f },
			Tools::BodyParams().position(startPosition).angle(startAngle).bodyType(b2_dynamicBody).density(0.2f)), force);

		auto& body = *missile.body;
		SetCollisionFilteringBits(body, Globals::CollisionBits::missile, Globals::CollisionBits::all - Globals::CollisionBits::missile - Globals::CollisionBits::plane);
		body.SetLinearVelocity(ToVec2<b2Vec2>(referenceVelocity + initialVelocity));
		body.SetBullet(true);

		missile.texture = TCM::Texture(missileTexture);
		missile.preserveTextureRatio = true;

		Globals::Components().renderingSetups().emplace([
			modelUniform = Uniforms::UniformMat4f(), &body](Shaders::ProgramId program) mutable
			{
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformMat4f(program, "model");
				modelUniform(Tools::GetModelMatrix(body));
				return nullptr;
			});

		missile.renderingSetup = Globals::Components().renderingSetups().size() - 1;
		missile.renderLayer = RenderLayer::FarMidground;

		auto& decoration = Globals::Components().dynamicDecorations().emplace(Tools::CreateVerticesOfRectangle({ 0.0f, -0.5f }, { 0.5f, 0.5f }),
			TCM::AnimatedTexture(flameAnimatedTexture), Tools::CreateTexCoordOfRectangle());

		Globals::Components().renderingSetups().emplace([&, modelUniform = Uniforms::UniformMat4f(),
			thrustScale = 0.1f
		](Shaders::ProgramId program) mutable {
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformMat4f(program, "model");
				modelUniform(glm::scale(glm::rotate(glm::translate(Tools::GetModelMatrix(*missile.body),
					{ -0.5f, 0.0f, 0.0f }),
					-glm::half_pi<float>(), { 0.0f, 0.0f, 1.0f }),
					{ std::min(thrustScale * 0.2f, 0.3f), thrustScale, 1.0f }));

				const float targetFrameDurationFactor = Globals::Components().physics().frameDuration * 6.0f;
				thrustScale = std::min(thrustScale * (1.0f + targetFrameDurationFactor), 3.0f);

				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
			});

		decoration.renderingSetup = Globals::Components().renderingSetups().size() - 1;
		decoration.renderLayer = RenderLayer::FarMidground;

		std::optional<ComponentId> soundId;
		if (missileSoundBuffer)
		{
			soundId = Tools::PlaySingleSound(*missileSoundBuffer,
				[&]() {
					return missile.getCenter();
				},
				[](auto& sound) {
					sound.removeOnStop(false);
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

			Globals::Components().renderingSetups().emplace([=, startTime = Globals::Components().physics().simulationDuration,
				&particlesShaders](Shaders::ProgramId program) mutable
				{
					particlesShaders.vp(Globals::Components().mvp().getVP());
					particlesShaders.texture1(params.explosionTexture_);

					const float elapsed = Globals::Components().physics().simulationDuration - startTime;
					particlesShaders.color(glm::vec4(glm::vec3(glm::pow(1.0f - elapsed / (params.explosionDuration_ * 2.0f), 10.0f)), 1.0f));

					glBlendFunc(GL_SRC_ALPHA, GL_ONE);

					return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
				});

			explosionDecoration.renderingSetup = Globals::Components().renderingSetups().size() - 1;
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

	void CreateFogForeground(int numOfLayers, float alphaPerLayer, ComponentId fogTexture,
		std::function<glm::vec4()> fColor)
	{
		for (int layer = 0; layer < numOfLayers; ++layer)
		for (int posYI = -1; posYI <= 1; ++posYI)
		for (int posXI = -1; posXI <= 1; ++posXI)
		{
			Globals::Components().renderingSetups().emplace([=, texturedProgram = Shaders::Programs::TexturedAccessor()
			](Shaders::ProgramId program) mutable {
				if (!texturedProgram.isValid()) texturedProgram = program;
				texturedProgram.vp(glm::translate(glm::scale(Globals::Components().mvp().getVP(), glm::vec3(glm::vec2(100.0f), 0.0f)),
					glm::vec3(-Globals::Components().camera().details.prevPosition * (0.002f + layer * 0.002f), 0.0f)));
				texturedProgram.color(fColor()* glm::vec4(1.0f, 1.0f, 1.0f, alphaPerLayer));

				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				return [texturedProgram]() mutable {
					texturedProgram.vp(Globals::Components().mvp().getVP());
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				};
			});

			Globals::Components().staticDecorations().emplace(Tools::CreateVerticesOfRectangle({ posXI, posYI }, glm::vec2(2.0f, 2.0f) + (layer * 0.2f)),
				TCM::Texture(fogTexture), Tools::CreateTexCoordOfRectangle(), Globals::Components().renderingSetups().size() - 1).renderLayer = RenderLayer::Foreground;
			Globals::Components().staticDecorations().last().resolutionMode = ResolutionMode::LowestLinearBlend1;
		}
	}

	void CreateJuliaBackground(std::function<glm::vec2()> juliaCOffset)
	{
		auto& juliaShaders = Globals::Shaders().julia();
		auto& background = Globals::Components().staticDecorations().emplace(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 10.0f, 10.0f }));
		background.customShadersProgram = juliaShaders.getProgramId();

		Globals::Components().renderingSetups().emplace([=, &juliaShaders
			](auto) mutable {
				juliaShaders.vp(glm::translate(glm::scale(glm::mat4(1.0f),
					glm::vec3((float)Globals::Components().screenInfo().windowSize.y / Globals::Components().screenInfo().windowSize.x, 1.0f, 1.0f) * 1.5f),
					glm::vec3(-Globals::Components().camera().details.prevPosition * 0.005f, 0.0f)));
				juliaShaders.juliaCOffset(juliaCOffset());
				juliaShaders.minColor({ 0.0f, 0.0f, 0.0f, 1.0f });
				juliaShaders.maxColor({ 0, 0.1f, 0.2f, 1.0f });
				return nullptr;
			});

		background.renderingSetup = Globals::Components().renderingSetups().size() - 1;
		background.renderLayer = RenderLayer::Background;
		background.resolutionMode = ResolutionMode::LowerLinearBlend1;
	}

	glm::vec2 GetRelativePos(glm::vec2 scenePos, bool projectionSizeScaling)
	{
		const auto& camera = Globals::Components().camera();
		return (scenePos - camera.details.position) / (projectionSizeScaling ? camera.details.projectionHSize : 1.0f);
	}

	Components::Sound& PlaySingleSound(ComponentId soundBuffer, std::function<glm::vec2()> posF,
		std::function<void(Components::Sound&)> config, std::function<void(Components::Sound&)> stepF)
	{
		auto& sound = Globals::Components().sounds().emplace(soundBuffer);

		sound.removeOnStop(true);
		sound.stepF = [posF = std::move(posF), stepF = std::move(stepF)](auto& sound)
		{
			if (posF)
				sound.position(Tools::GetRelativePos(posF()));
			if (stepF)
				stepF(sound);
		};

		if (config)
			config(sound);

		sound.play();

		return sound;
	}
}
