#include "gameHelpers.hpp"

#include <globals.hpp>

#include <systems/deferredActions.hpp>

#include <components/player.hpp>
#include <components/physics.hpp>
#include <components/decoration.hpp>
#include <components/missile.hpp>
#include <components/shockwave.hpp>
#include <components/renderingSetup.hpp>
#include <components/mvp.hpp>
#include <components/camera.hpp>
#include <components/screenInfo.hpp>
#include <components/animationTexture.hpp>

#include <ogl/uniformControllers.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/shaders/julia.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/utility.hpp>

#include <collisionBits.hpp>

#include <commonTypes/typeComponentMappers.hpp>

#include <cassert>

namespace Tools
{
	PlayerPlaneHandler CreatePlayerPlane(unsigned planeTexture, unsigned flameAnimationTexture)
	{
		PlayerPlaneHandler playerPlaneHandler;

		playerPlaneHandler.playerId = Globals::Components().players().size();
		auto& player = Globals::Components().players().emplace_back(Tools::CreateTrianglePlayerBody(2.0f, 0.2f), TCM::Texture(planeTexture));
		SetCollisionFilteringBits(*player.body, CollisionBits::playerBit, CollisionBits::all);
		player.setPosition({ -10.0f, 0.0f });

		Globals::Components().renderingSetups().emplace_back([
			colorUniform = Uniforms::UniformController4f()
			](Shaders::ProgramId program) mutable {
					if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
					const float fade = (glm::sin(Globals::Components().physics().simulationDuration * 2.0f * glm::two_pi<float>()) + 1.0f) / 2.0f;
					colorUniform({ fade, 1.0f, fade, 1.0f });
					return nullptr;
				});

		player.renderingSetup = Globals::Components().renderingSetups().size() - 1;

		for (int i = 0; i < 2; ++i)
		{
			const unsigned animationTextureId = Globals::Components().animationTextures().size();
			Globals::Components().animationTextures().push_back(Components::AnimationTexture(
				flameAnimationTexture, { 500, 498 }, { 2, 0 }, { 61, 120 }, { 8, 4 }, { 62.5f, 124.9f }, 0.02f, 0,
				AnimationLayout::Horizontal, AnimationPlayback::Backward, AnimationPolicy::Repeat,
				{ 0.0f, -0.45f }, { 1.0f, 1.0f }));
			auto& animationTexture = Globals::Components().animationTextures().back();

			playerPlaneHandler.backThrustsIds[i] = Globals::Components().farMidgroundDecorations().size();
			auto& decoration = Globals::Components().farMidgroundDecorations().emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, -0.45f }, { 0.5f, 0.5f }),
				TCM::AnimationTexture(animationTextureId));

			Globals::Components().renderingSetups().emplace_back([&, i, modelUniform = Uniforms::UniformControllerMat4f(),
				thrustScale = 1.0f
				](Shaders::ProgramId program) mutable {
					if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
					modelUniform(glm::scale(glm::rotate(glm::translate(Tools::GetModelMatrix(*player.body),
						{ -0.9f, i == 0 ? -0.42f : 0.42f, 0.0f }),
						-glm::half_pi<float>() + (i == 0 ? 0.1f : -0.1f), { 0.0f, 0.0f, 1.0f }),
						{ std::min(thrustScale * 0.5f, 0.7f), thrustScale, 1.0f }));

					const float targetFrameDurationFactor = Globals::Components().physics().frameDuration * 6;
					if (player.throttling) thrustScale = std::min(thrustScale * (1.0f + targetFrameDurationFactor), 5.0f);
					else thrustScale = 1.0f + (thrustScale - 1.0f) * (1.0f - targetFrameDurationFactor);

					glBlendFunc(GL_SRC_ALPHA, GL_ONE);

					return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
				});

			decoration.renderingSetup = Globals::Components().renderingSetups().size() - 1;

			animationTexture.start();
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

		auto missileIt = Globals::Components().missiles().find(missileId);
		assert(missileIt != Globals::Components().missiles().end());
		missileIt->second.state = ComponentState::Outdated;
		auto thrustIt = Globals::Components().temporaryFarMidgroundDecorations().find(backThrustId);
		assert(thrustIt != Globals::Components().temporaryFarMidgroundDecorations().end());
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

	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 initialVelocity,
		unsigned missileTexture, unsigned flameAnimationTexture)
	{
		auto& missile = EmplaceIdComponent(Globals::Components().missiles(), { Tools::CreateBoxBody(startPosition, { 0.5f, 0.2f }, startAngle, b2_dynamicBody, 0.2f) });
		auto& body = *missile.body;
		SetCollisionFilteringBits(body, CollisionBits::missileBit, CollisionBits::all - CollisionBits::missileBit - CollisionBits::playerBit);
		body.SetBullet(true);
		body.SetLinearVelocity({ initialVelocity.x, initialVelocity.y });
		missile.texture = TCM::Texture(missileTexture);

		Globals::Components().renderingSetups().emplace_back([
			modelUniform = Uniforms::UniformControllerMat4f(), &body](Shaders::ProgramId program) mutable
			{
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
				modelUniform(Tools::GetModelMatrix(body));
				return nullptr;
			});

		missile.renderingSetup = Globals::Components().renderingSetups().size() - 1;

		missile.step = [&body, force, launchTime = Globals::Components().physics().simulationDuration, fullCollisions = false]() mutable
		{
			if (!fullCollisions && (Globals::Components().physics().simulationDuration - launchTime) > 0.5f)
			{
				SetCollisionFilteringBits(body, CollisionBits::missileBit, CollisionBits::all);
				fullCollisions = true;
			}
			body.ApplyForceToCenter({ glm::cos(body.GetAngle()) * force, glm::sin(body.GetAngle()) * force }, true);
		};

		const unsigned animationTextureId = Globals::Components().animationTextures().size();
		Globals::Components().animationTextures().push_back(Components::AnimationTexture(
			flameAnimationTexture, { 500, 498 }, { 2, 0 }, { 61, 120 }, { 8, 4 }, { 62.5f, 124.9f }, 0.02f, 0,
			AnimationLayout::Horizontal, AnimationPlayback::Backward, AnimationPolicy::Repeat,
			{ 0.0f, -0.45f }, { 1.0f, 1.0f }));
		auto& animationTexture = Globals::Components().animationTextures().back();

		auto& decoration = EmplaceIdComponent(Globals::Components().temporaryFarMidgroundDecorations(), { Tools::CreatePositionsOfRectangle({ 0.0f, -0.45f }, { 0.5f, 0.5f }),
			TCM::AnimationTexture(animationTextureId) });

		Globals::Components().renderingSetups().emplace_back([&, modelUniform = Uniforms::UniformControllerMat4f(),
			thrustScale = 0.1f
			](Shaders::ProgramId program) mutable {
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
				modelUniform(glm::scale(glm::rotate(glm::translate(Tools::GetModelMatrix(*missile.body),
					{ -0.65f, 0.0f, 0.0f }),
					-glm::half_pi<float>() + 0.0f, { 0.0f, 0.0f, 1.0f }),
					{ std::min(thrustScale * 0.2f, 0.4f), thrustScale, 1.0f }));

				const float targetFrameDurationFactor = Globals::Components().physics().frameDuration * 10;
				thrustScale = std::min(thrustScale * (1.0f + targetFrameDurationFactor), 3.0f);

				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
			});

		decoration.renderingSetup = Globals::Components().renderingSetups().size() - 1;

		animationTexture.start();

		return { missile.getComponentId(), decoration.getComponentId() };
	}

	void CreateExplosion(Shaders::Programs::ParticlesAccessor particlesProgram, glm::vec2 center, unsigned explosionTexture,
		float explosionDuration, int numOfParticles, int particlesPerDecoration, ResolutionMode resolutionMode)
	{
		Globals::Systems().deferredActions().addDeferredAction([=]() {
			auto& shockwave = EmplaceIdComponent(Globals::Components().shockwaves(), { center, numOfParticles });
			auto& explosionDecoration = EmplaceIdComponent(Globals::Components().temporaryNearMidgroundDecorations(), {});
			explosionDecoration.customShadersProgram = particlesProgram.getProgramId();
			explosionDecoration.resolutionMode = resolutionMode;
			explosionDecoration.drawMode = GL_POINTS;
			explosionDecoration.bufferDataUsage = GL_DYNAMIC_DRAW;

			Globals::Components().renderingSetups().emplace_back([=, startTime = Globals::Components().physics().simulationDuration
				](Shaders::ProgramId program) mutable
				{
					particlesProgram.vpUniform(Globals::Components().mvp().getVP());
					particlesProgram.texture1Uniform(explosionTexture);

					const float elapsed = Globals::Components().physics().simulationDuration - startTime;
					particlesProgram.colorUniform(glm::vec4(glm::vec3(glm::pow(1.0f - elapsed / (explosionDuration * 2.0f), 10.0f)), 1.0f));

					glBlendFunc(GL_SRC_ALPHA, GL_ONE);

					return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
				});

			explosionDecoration.renderingSetup = Globals::Components().renderingSetups().size() - 1;

			Globals::Systems().deferredActions().addDeferredAction([=, startTime = Globals::Components().physics().simulationDuration, &shockwave, &explosionDecoration]() {
				const float elapsed = Globals::Components().physics().simulationDuration - startTime;
				const float scale = 1.0f + elapsed * 20.0f;

				if (elapsed > explosionDuration)
				{
					shockwave.state = ComponentState::Outdated;
					explosionDecoration.state = ComponentState::Outdated;
					return false;
				}

				explosionDecoration.positions.clear();
				explosionDecoration.positions.emplace_back(shockwave.center, scale);
				for (size_t i = 0; i < shockwave.particles.size(); ++i)
				{
					if (i % particlesPerDecoration != 0) continue;
					const auto& particle = shockwave.particles[i];
					const glm::vec2 position = shockwave.center + (ToVec2<glm::vec2>(particle->GetWorldCenter()) - shockwave.center) * 0.5f;
					explosionDecoration.positions.emplace_back(position, scale);
				}
				explosionDecoration.state = ComponentState::Changed;

				return true;
			});

			return false;
			});
	}

	void CreateFogForeground(int numOfLayers, float alphaPerLayer, unsigned fogTexture,
		std::function<glm::vec4()> fColor)
	{
		for (int layer = 0; layer < numOfLayers; ++layer)
		for (int posYI = -1; posYI <= 1; ++posYI)
		for (int posXI = -1; posXI <= 1; ++posXI)
		{
			Globals::Components().foregroundDecorations().emplace_back(Tools::CreatePositionsOfRectangle({ posXI, posYI }, glm::vec2(2.0f, 2.0f) + (layer * 0.2f)), TCM::Texture(fogTexture));
			Globals::Components().foregroundDecorations().back().texCoord = Tools::CreateTexCoordOfRectangle();

			Globals::Components().renderingSetups().emplace_back([=, texturedProgram = Shaders::Programs::TexturedAccessor()
				](Shaders::ProgramId program) mutable {
					if (!texturedProgram.isValid()) texturedProgram = program;
					texturedProgram.vpUniform(glm::translate(glm::scale(Globals::Components().mvp().getVP(), glm::vec3(glm::vec2(100.0f), 0.0f)),
						glm::vec3(-Globals::Components().camera().prevPosition * (0.002f + layer * 0.002f), 0.0f)));
					texturedProgram.colorUniform(fColor()* glm::vec4(1.0f, 1.0f, 1.0f, alphaPerLayer));

					glBlendFunc(GL_SRC_ALPHA, GL_ONE);

					return [texturedProgram]() mutable {
						texturedProgram.vpUniform(Globals::Components().mvp().getVP());
						glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
					};
				});

			Globals::Components().foregroundDecorations().back().renderingSetup = Globals::Components().renderingSetups().size() - 1;
			Globals::Components().foregroundDecorations().back().resolutionMode = ResolutionMode::LowestLinearBlend1;
		}
	}

	void CreateJuliaBackground(Shaders::Programs::Julia& juliaShaders, std::function<glm::vec2()> juliaCOffset)
	{
		auto& background = Globals::Components().backgroundDecorations().emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 10.0f, 10.0f }));
		background.customShadersProgram = juliaShaders.getProgramId();

		Globals::Components().renderingSetups().emplace_back([=, &juliaShaders
			](auto) mutable {
				juliaShaders.vpUniform(glm::translate(glm::scale(glm::mat4(1.0f),
					glm::vec3((float)Globals::Components().screenInfo().windowSize.y / Globals::Components().screenInfo().windowSize.x, 1.0f, 1.0f) * 1.5f),
					glm::vec3(-Globals::Components().camera().prevPosition * 0.005f, 0.0f)));
				juliaShaders.juliaCOffsetUniform(juliaCOffset());
				juliaShaders.minColorUniform({ 0.0f, 0.0f, 0.0f, 1.0f });
				juliaShaders.maxColorUniform({ 0, 0.1f, 0.2f, 1.0f });
				return nullptr;
			});

		background.renderingSetup = Globals::Components().renderingSetups().size() - 1;
		background.resolutionMode = ResolutionMode::LowerLinearBlend1;
	}
}
