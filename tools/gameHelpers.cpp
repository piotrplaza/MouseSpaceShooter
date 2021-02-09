#include "gameHelpers.hpp"

#include <cassert>

#include <globals.hpp>

#include <systems/deferredActions.hpp>

#include <components/player.hpp>
#include <components/physics.hpp>
#include <components/decoration.hpp>
#include <components/missile.hpp>
#include <components/shockwave.hpp>
#include <components/mvp.hpp>

#include <ogl/uniformControllers.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/animations.hpp>
#include <tools/b2Helpers.hpp>
#include <tools/utility.hpp>

#include <collisionBits.hpp>

namespace Tools
{
	PlayerPlaneHandler CreatePlayerPlane(unsigned planeTexture, unsigned flameAnimationTexture)
	{
		using namespace Globals::Components;

		PlayerPlaneHandler playerPlaneHandler;

		playerPlaneHandler.playerId = players.size();
		auto& player = players.emplace_back(Tools::CreateTrianglePlayerBody(2.0f, 0.2f), planeTexture);
		SetCollisionFilteringBits(*player.body, CollisionBits::playerBit, CollisionBits::all);
		player.setPosition({ -10.0f, 0.0f });
		player.renderingSetup = std::make_unique<Components::Player::RenderingSetup>([
			colorUniform = Uniforms::UniformController4f()
		](Shaders::ProgramId program) mutable {
			if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
			const float fade = (glm::sin(physics.simulationTime * 2.0f * glm::two_pi<float>()) + 1.0f) / 2.0f;
			colorUniform.setValue({ fade, 1.0f, fade, 1.0f });
			return nullptr;
		});

		for (int i = 0; i < 2; ++i)
		{
			playerPlaneHandler.backThrustsIds[i] = backgroundDecorations.size();
			auto& decoration = backgroundDecorations.emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, -0.45f }, { 0.5f, 0.5f }), flameAnimationTexture);

			decoration.renderingSetup = std::make_unique<Components::Decoration::RenderingSetup>([&, i, modelUniform = Uniforms::UniformControllerMat4f(),
				thrustScale = 1.0f
			](Shaders::ProgramId program) mutable {
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
				modelUniform.setValue(glm::scale(glm::rotate(glm::translate(Tools::GetModelMatrix(*player.body),
					{ -0.9f, i == 0 ? -0.42f : 0.42f, 0.0f }),
					-glm::half_pi<float>() + (i == 0 ? 0.1f : -0.1f), { 0.0f, 0.0f, 1.0f }),
					{ std::min(thrustScale * 0.5f, 0.7f), thrustScale, 1.0f }));

				const float targetFrameTimeFactor = physics.frameTime * 6;
				if (player.throttling) thrustScale = std::min(thrustScale * (1.0f + targetFrameTimeFactor), 5.0f);
				else thrustScale = 1.0f + (thrustScale - 1.0f) * (1.0f - targetFrameTimeFactor);

				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				return []() { glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); };
			});

			decoration.animationController.reset(new Tools::TextureAnimationController(
				{ 500, 498 }, { 2, 0 }, { 61, 120 }, { 8, 4 }, { 62.5f, 124.9f }, 0.02f, 0,
				AnimationLayout::Horizontal, AnimationPlayback::Backward, AnimationPolicy::Repeat,
				{ 0.0f, -0.45f }, { 1.0f, 1.0f }));

			decoration.animationController->start();
		}

		return playerPlaneHandler;
	}

	MissileHandler::MissileHandler() = default;

	MissileHandler::MissileHandler(ComponentId missileId, ComponentId backThrustId):
		missileId(missileId),
		backThrustId(backThrustId)
	{
	}

	MissileHandler::~MissileHandler()
	{
		if (!valid) return;

		using namespace Globals::Components;

		auto missileIt = missiles.find(missileId);
		assert(missileIt != missiles.end());
		missileIt->second.state = ComponentState::Outdated;
		auto thrustIt = temporaryBackgroundDecorations.find(backThrustId);
		assert(thrustIt != temporaryBackgroundDecorations.end());
		thrustIt->second.state = ComponentState::Outdated;
	}

	MissileHandler::MissileHandler(MissileHandler&& other) noexcept:
		missileId(other.missileId),
		backThrustId(other.backThrustId)
	{
		other.valid = false;
	}

	MissileHandler& MissileHandler::operator=(MissileHandler&& other) noexcept
	{
		missileId = other.missileId;
		backThrustId = other.backThrustId;
		other.valid = false;
		return *this;
	}

	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 initialVelocity, unsigned missileTexture, unsigned flameAnimationTexture)
	{
		using namespace Globals::Components;

		auto& missile = EmplaceIdComponent(missiles, { Tools::CreateBoxBody(startPosition, { 0.5f, 0.2f }, startAngle, b2_dynamicBody, 0.2f) });
		auto& body = *missile.body;
		SetCollisionFilteringBits(body, CollisionBits::missileBit, CollisionBits::all - CollisionBits::missileBit - CollisionBits::playerBit);
		body.SetBullet(true);
		body.SetLinearVelocity({ initialVelocity.x, initialVelocity.y });
		missile.texture = missileTexture;
		missile.renderingSetup = std::make_unique<Components::Missile::RenderingSetup>(
			[modelUniform = Uniforms::UniformControllerMat4f(), &body](Shaders::ProgramId program) mutable
		{
			if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
			modelUniform.setValue(Tools::GetModelMatrix(body));
			return nullptr;
		});
		missile.step = [&body, force, launchTime = physics.simulationTime, fullCollisions = false]() mutable
		{
			if (!fullCollisions && (physics.simulationTime - launchTime) > 0.5f)
			{
				SetCollisionFilteringBits(body, CollisionBits::missileBit, CollisionBits::all);
				fullCollisions = true;
			}
			body.ApplyForceToCenter({ glm::cos(body.GetAngle()) * force, glm::sin(body.GetAngle()) * force }, true);
		};

		auto& decoration = EmplaceIdComponent(temporaryBackgroundDecorations, { Tools::CreatePositionsOfRectangle({ 0.0f, -0.45f }, { 0.5f, 0.5f }), flameAnimationTexture });
		decoration.renderingSetup = std::make_unique<Components::Decoration::RenderingSetup>([&, modelUniform = Uniforms::UniformControllerMat4f(),
			thrustScale = 0.1f
		](Shaders::ProgramId program) mutable {
			if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
			modelUniform.setValue(glm::scale(glm::rotate(glm::translate(Tools::GetModelMatrix(*missile.body),
				{ -0.65f, 0.0f, 0.0f }),
				-glm::half_pi<float>() + 0.0f, { 0.0f, 0.0f, 1.0f }),
				{ std::min(thrustScale * 0.2f, 0.4f), thrustScale, 1.0f }));

			const float targetFrameTimeFactor = physics.frameTime * 10;
			thrustScale = std::min(thrustScale * (1.0f + targetFrameTimeFactor), 3.0f);

			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			return []() { glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); };
		});

		decoration.animationController.reset(new Tools::TextureAnimationController(
			{ 500, 498 }, { 2, 0 }, { 61, 120 }, { 8, 4 }, { 62.5f, 124.9f }, 0.02f, 0,
			AnimationLayout::Horizontal, AnimationPlayback::Backward, AnimationPolicy::Repeat,
			{ 0.0f, -0.45f }, { 1.0f, 1.0f }));

		decoration.animationController->start();

		return { missile.componentId, decoration.componentId };
	}

	void CreateExplosion(Shaders::Programs::ParticlesAccessor particlesProgram, glm::vec2 center, unsigned explosionTexture, float explosionDuration, int numOfParticles, int particlesPerDecoration)
	{
		using namespace Globals::Components;

		Globals::Systems::DeferredActions().addDeferredAction([=]() {
			auto& shockwave = EmplaceIdComponent(shockwaves, { center, numOfParticles });
			auto& explosionDecoration = EmplaceIdComponent(temporaryForegroundDecorations, { std::vector<glm::vec3>{} });
			explosionDecoration.customShadersProgram = particlesProgram.program;
			explosionDecoration.drawMode = GL_POINTS;
			explosionDecoration.bufferDataUsage = GL_DYNAMIC_DRAW;
			explosionDecoration.renderingSetup = std::make_unique<Components::Decoration::RenderingSetup>(
				[=, startTime = physics.simulationTime](Shaders::ProgramId program) mutable
			{
				particlesProgram.vpUniform.setValue(mvp.getVP());
				particlesProgram.texture1Uniform.setValue(explosionTexture);

				const float elapsed = physics.simulationTime - startTime;
				particlesProgram.colorUniform.setValue(glm::vec4(glm::vec3(glm::pow(1.0f - elapsed / (explosionDuration * 2.0f), 10.0f)), 1.0f));

				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				return []() { glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); };
			});
			Globals::Systems::DeferredActions().addDeferredAction([=, startTime = physics.simulationTime, &shockwave, &explosionDecoration]() {
				const float elapsed = physics.simulationTime - startTime;

				if (elapsed > explosionDuration)
				{
					shockwave.state = ComponentState::Outdated;
					explosionDecoration.state = ComponentState::Outdated;
					return false;
				}

				explosionDecoration.positions.clear();
				explosionDecoration.positions.push_back(glm::vec3(shockwave.center, 1.0f));
				for (size_t i = 0; i < shockwave.particles.size(); ++i)
				{
					if (i % particlesPerDecoration != 0) continue;
					const auto& particle = shockwave.particles[i];
					const glm::vec2 position = shockwave.center + (ToVec2<glm::vec2>(particle->GetWorldCenter()) - shockwave.center) * 0.5f;
					const float scale = 1.0f + elapsed * 20.0f;
					explosionDecoration.positions.push_back(glm::vec3(position, scale));
				}
				explosionDecoration.state = ComponentState::Changed;

				return true;
			});

			return false;
			});
	}
}
