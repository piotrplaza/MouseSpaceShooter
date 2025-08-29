#include "gameHelpers.hpp"

#include <systems/deferredActions.hpp>

#include <components/plane.hpp>
#include <components/physics.hpp>
#include <components/decoration.hpp>
#include <components/missile.hpp>
#include <components/vp.hpp>
#include <components/camera2D.hpp>
#include <components/systemInfo.hpp>
#include <components/animatedTexture.hpp>
#include <components/sound.hpp>
#include <components/audioListener.hpp>
#include <components/texture.hpp>

#include <commonTypes/componentMappers.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>
#include <globals/collisionBits.hpp>

#include <ogl/shaders/textured.hpp>
#include <ogl/shaders/julia.hpp>

#include <tools/Shapes2D.hpp>

#include <set>

namespace Tools
{
	MissileHandler::MissileHandler() = default;

	MissileHandler::MissileHandler(CM::Missile missile, CM::Decoration backThrust, glm::vec2 referenceVelocity, std::optional<CM::Plane> plane,
		std::optional<CM::Sound> sound):
		missile(missile),
		backThrust(backThrust),
		referenceVelocity(referenceVelocity),
		plane(plane),
		sound(sound)
	{
	}

	MissileHandler::~MissileHandler()
	{
		if (!valid)
			return;

		missile.component->state = ComponentState::Outdated;
		backThrust.component->state = ComponentState::Outdated;

		if (sound)
			sound->component->state = ComponentState::Outdated;
	}

	MissileHandler::MissileHandler(MissileHandler&& other) noexcept:
		missile(other.missile),
		backThrust(other.backThrust),
		referenceVelocity(other.referenceVelocity),
		plane(other.plane),
		sound(other.sound),
		collided(other.collided)
	{
		other.valid = false;
	}

	MissileHandler& MissileHandler::operator=(MissileHandler&& other) noexcept
	{
		missile = other.missile;
		backThrust = other.backThrust;
		referenceVelocity = other.referenceVelocity;
		plane = other.plane;
		sound = other.sound;
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

	Components::Sound& SoundsLimitter::newSound(Components::Sound& sound, std::function<void()> tearDown)
	{
		if (limit == 0)
		{
			sound.immediateFreeResources();
			sound.soundTeardownF = [tearDown]() {
				if (tearDown)
					tearDown();
			};
			return sound;
		}

		if (sounds.size() == limit)
			sounds.front()->immediateFreeResources();

		sounds.push_back(&sound);

		sound.soundTeardownF = [this, soundsLimitter = shared_from_this(), it = std::prev(sounds.end()), tearDown]() {
			sounds.erase(it);
			if (tearDown)
				tearDown();
		};

		return sound;
	}

	const Tools::BodyParams& GetDefaultParamsForPlaneBody()
	{
		static const auto planeBodyDefaultParams = Tools::BodyParams().bodyType(b2_dynamicBody).density(0.2f).restitution(0.1f).autoSleeping(false).bullet(true).linearDamping(0.1f).angularDamping(15.0);
		return planeBodyDefaultParams;
	}

	ComponentId CreatePlane(Body body, CM::Texture planeTexture, CM::AnimatedTexture thrustAnimatedTexture, PlaneParams params)
	{
		auto& plane = Globals::Components().planes().emplace(std::move(body), planeTexture);

		plane.setOrigin(params.position_);
		plane.setAngle(params.angle_);
		plane.posInSubsequence = params.numOfThrusts_ + params.collisionBoxRendering_;

		if (params.collisionBoxRendering_)
		{
			plane.subsequence.emplace_back(plane.getPositions());
			plane.subsequence.back().modelMatrixF = plane.modelMatrixF;
		}

		for (int i = 0; i < params.numOfThrusts_; ++i)
		{
			auto& planeDecoration = plane.subsequence.emplace_back(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 0.5f, 0.5f }), std::vector<glm::vec2>{}, thrustAnimatedTexture);

			planeDecoration.renderingSetupF = [&, i,
				modelUniform = UniformsUtils::UniformMat4f(),
				thrust = 1.0f,
				thrustAnimatedTexture,
				params
			](ShadersUtils::ProgramId program) mutable {
				if (!modelUniform.isValid())
					modelUniform.reset(program, "model");
				
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

				thrustAnimatedTexture.component->setSpeedScaling(1.0f + (thrust - 1) * 0.2f);

				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
			};
		}

		return plane.getComponentId();
	}

	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 referenceVelocity,
		glm::vec2 initialVelocity, CM::Texture missileTexture, CM::AnimatedTexture thrustAnimatedTexture, std::optional<CM::Plane> plane,
		std::optional<CM::SoundBuffer> missileSoundBuffer)
	{
		auto& missile = Globals::Components().missiles().emplace(Tools::CreateBoxBody({ 0.5f, 0.2f },
			Tools::BodyParams().position(startPosition).angle(startAngle).bodyType(b2_dynamicBody).density(0.2f)), force);

		auto& body = *missile.body;
		SetCollisionFilteringBits(body, Globals::CollisionBits::projectile, Globals::CollisionBits::all - Globals::CollisionBits::projectile - Globals::CollisionBits::actor);
		body.SetLinearVelocity(ToVec2<b2Vec2>(referenceVelocity + initialVelocity));
		body.SetBullet(true);

		missile.texture = missileTexture;

		missile.renderingSetupF = [modelUniform = UniformsUtils::UniformMat4f(), &body](ShadersUtils::ProgramId program) mutable {
			if (!modelUniform.isValid())
				modelUniform.reset(program, "model");
			modelUniform(Tools::GetModelMatrix(body));
			return nullptr;
		};

		missile.renderLayer = RenderLayer::FarMidground;

		auto& decoration = Globals::Components().decorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, -0.5f }, { 0.5f, 0.5f }),
			thrustAnimatedTexture, Tools::Shapes2D::CreateTexCoordOfRectangle());

		decoration.renderingSetupF = [&, modelUniform = UniformsUtils::UniformMat4f(), thrustScale = 0.1f](ShadersUtils::ProgramId program) mutable {
			if (!modelUniform.isValid())
				modelUniform.reset(program, "model");
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

		std::optional<CM::Sound> sound;
		if (missileSoundBuffer)
		{
			sound = Tools::CreateAndPlaySound(*missileSoundBuffer,
				[&]() {
					return missile.getOrigin2D();
				},
				[](auto& sound) {
					sound.setRemoveOnStop(false);
				});
		}

		return { missile.getComponentId(), decoration, referenceVelocity, plane, sound };
	}

	void CreateFogForeground(int numOfLayers, float alphaPerLayer, CM::Texture fogTexture, FVec4 fColor, std::function<glm::vec2(int layer)> textureTranslation)
	{
		for (int layer = 0; layer < numOfLayers; ++layer)
		for (int posYI = -1; posYI <= 1; ++posYI)
		for (int posXI = -1; posXI <= 1; ++posXI)
		{
			auto renderingSetupF = [=, texturedProgram = ShadersUtils::Programs::TexturedAccessor()
			](ShadersUtils::AccessorBase& shaderBase) mutable {
				auto& program = static_cast<ShadersUtils::Programs::TexturedAccessor&>(shaderBase);
				program.vp(glm::translate(glm::scale(Globals::Components().vpDefault2D().getVP(), glm::vec3(glm::vec2(100.0f), 0.0f)),
					glm::vec3(-Globals::Components().camera2D().details.prevPosition * (0.0002f + layer * 0.0002f), 0.0f)));
				program.color(fColor() * glm::vec4(1.0f, 1.0f, 1.0f, alphaPerLayer));

				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				return [&]() mutable {
					program.vp(Globals::Components().vpDefault2D().getVP());
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				};
			};

			auto& fogLayer = Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ posXI, posYI }, glm::vec2(2.0f, 2.0f) + (layer * 0.2f)),
				CM::DummyTexture(), Tools::Shapes2D::CreateTexCoordOfRectangle(), std::move(renderingSetupF));
			fogLayer.targetTexture = Globals::Components().standardRenderTexture({ StandardRenderMode::Resolution::H540, StandardRenderMode::Scaling::Linear, StandardRenderMode::mainBlending });
			fogLayer.renderLayer = RenderLayer::Foreground;
			fogLayer.stepF = [=, &fogLayer]() mutable {
				fogTexture.translate = textureTranslation ? textureTranslation(layer) : glm::vec2(0.0f);
				fogLayer.texture = fogTexture;
			};
		}
	}

	Components::Decoration& CreateJuliaBackground(JuliaParams params)
	{
		auto& juliaShaders = Globals::Shaders().julia();
		auto& background = Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 10.0f, 10.0f }));
		background.customShadersProgram = &juliaShaders;

		background.renderingSetupF = [params, &juliaShaders, &screenInfo = Globals::Components().systemInfo().screen](auto&) {
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
		background.targetTexture = Globals::Components().standardRenderTexture({ StandardRenderMode::Resolution::Native, StandardRenderMode::Scaling::Linear, StandardRenderMode::Blending::Additive });

		return background;
	}

	Components::Sound& CreateAndPlaySound(CM::SoundBuffer soundBuffer, FVec2 posF, std::function<void(Components::Sound&)> config, std::function<void(Components::Sound&)> stepF)
	{
		auto& sound = Globals::Components().sounds().emplace(soundBuffer);

		sound.setRemoveOnStop(true);

		if (posF.isLoaded())
			sound.setPosition(posF());

		if (config)
			config(sound);

		sound.stepF = [&, posF = std::move(posF), stepF = std::move(stepF)]() {
			if (posF.isLoaded())
				sound.setPosition(posF());

			if (stepF)
				stepF(sound);
		};

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

	void CountDown(CM::AnimatedTexture digitsAnimatedTexture, CM::SoundBuffer countSoundBufferId, CM::SoundBuffer startSoundBufferId, float onScreenScale, std::function<void()> startF)
	{
		const auto& physics = Globals::Components().physics();
		const auto& systemInfo = Globals::Components().systemInfo();
		auto& digit = Globals::Components().decorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, glm::vec2(0.7f / systemInfo.screen.getAspectRatio(), 1.0f) * onScreenScale), digitsAnimatedTexture, Tools::Shapes2D::CreateTexCoordOfRectangle());
		auto scale = std::make_shared<float>(1.0f);
		digit.renderingSetupF = [&, scale](ShadersUtils::AccessorBase& accessorBase) {
			auto& shader = static_cast<ShadersUtils::Programs::TexturedAccessor&>(accessorBase);
			shader.vp(glm::scale(glm::mat4(1.0f), glm::vec3(*scale)));
			return nullptr;
		};
		digit.stepF = [&, start = physics.simulationDuration, digitsAnimatedTexture, countSoundBufferId, startSoundBufferId, scale, startF = std::move(startF)]() mutable {
			const float duration = physics.simulationDuration - start;
			if (duration < 1.0f)
			{
				if (!digitsAnimatedTexture.component->getForcedFrame() || *digitsAnimatedTexture.component->getForcedFrame() != 3)
				{
					digitsAnimatedTexture.component->setForcedFrame(3);
					Tools::CreateAndPlaySound(countSoundBufferId);
				}
				digit.colorF = glm::vec4(1.0f - duration);
			}
			else if (duration < 2.0f)
			{
				if (*digitsAnimatedTexture.component->getForcedFrame() != 2)
				{
					digitsAnimatedTexture.component->setForcedFrame(2);
					Tools::CreateAndPlaySound(countSoundBufferId);
				}
				digit.colorF = glm::vec4(2.0f - duration);
			}
			else if (duration < 3.0f)
			{
				if (*digitsAnimatedTexture.component->getForcedFrame() != 1)
				{
					digitsAnimatedTexture.component->setForcedFrame(1);
					Tools::CreateAndPlaySound(countSoundBufferId);
				}
				digit.colorF = glm::vec4(3.0f - duration);
			}
			else if (duration < 4.0f)
			{
				if (*digitsAnimatedTexture.component->getForcedFrame() != 0)
				{
					digitsAnimatedTexture.component->setForcedFrame(0);
					Tools::CreateAndPlaySound(startSoundBufferId);
					if (startF)
						startF();
				}
				*scale = 1.0f + (duration - 3.0f) * 10.0f;
				digit.colorF = glm::vec4(1.0f - (duration - 3.0f) * 2.0f);
			}
			else
			{
				digitsAnimatedTexture.component->setForcedFrame(std::nullopt);
				digit.state = ComponentState::Outdated;
			}
			return true;
		};
	}
}
