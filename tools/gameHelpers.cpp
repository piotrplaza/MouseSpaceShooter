#include "gameHelpers.hpp"

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
#include <components/animatedTexture.hpp>

#include <components/typeComponentMappers.hpp>

#include <ogl/uniformControllers.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/shaders/julia.hpp>

#include <globals/systems.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/utility.hpp>

#include <commonIds/collisionBits.hpp>

#include <cassert>

namespace Tools
{
	PlayerPlaneHandler CreatePlayerPlane(unsigned planeTexture, unsigned flameAnimatedTexture, glm::vec2 position, float angle)
	{
		PlayerPlaneHandler playerPlaneHandler;

		playerPlaneHandler.playerId = Globals::Components().players().size();
		auto& player = Globals::Components().players().emplace_back(Tools::CreateTrianglePlayerBody(2.0f, 0.2f, 0.5f), TCM::Texture(planeTexture));
		SetCollisionFilteringBits(*player.body, CollisionBits::playerBit, CollisionBits::all);
		player.setPosition(position);
		player.setRotation(angle);
		player.preserveTextureRatio = true;

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
			auto& animationTexture = Globals::Components().animatedTextures().back();

			playerPlaneHandler.backThrustsIds[i] = Globals::Components().farMidgroundDecorations().size();
			auto& decoration = Globals::Components().farMidgroundDecorations().emplace_back(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 0.5f, 0.5f }),
				TCM::AnimatedTexture(flameAnimatedTexture));

			Globals::Components().renderingSetups().emplace_back([&, i, modelUniform = Uniforms::UniformControllerMat4f(),
				thrustScale = 1.0f
				](Shaders::ProgramId program) mutable {
					if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
					modelUniform(glm::scale(glm::rotate(glm::translate(Tools::GetModelMatrix(*player.body),
						{ -1.0f - thrustScale * 0.25f, i == 0 ? -0.5f : 0.5f, 0.0f }),
						-glm::half_pi<float>() + (i == 0 ? 0.1f : -0.1f), { 0.0f, 0.0f, 1.0f }),
						{ std::min(thrustScale * 0.5f, 0.6f), thrustScale, 1.0f }));

					const float targetFrameDurationFactor = Globals::Components().physics().frameDuration * 6;
					if (player.throttling) thrustScale = std::min(thrustScale * (1.0f + targetFrameDurationFactor), 5.0f);
					else thrustScale = 1.0f + (thrustScale - 1.0f) * (1.0f - targetFrameDurationFactor);

					glBlendFunc(GL_SRC_ALPHA, GL_ONE);

					return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
				});

			decoration.renderingSetup = Globals::Components().renderingSetups().size() - 1;
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
		unsigned missileTexture, unsigned flameAnimatedTexture)
	{
		auto& missile = EmplaceIdComponent(Globals::Components().missiles(), { Tools::CreateBoxBody(startPosition, { 0.5f, 0.2f }, startAngle, b2_dynamicBody, 0.2f) });
		auto& body = *missile.body;
		SetCollisionFilteringBits(body, CollisionBits::missileBit, CollisionBits::all - CollisionBits::missileBit - CollisionBits::playerBit);
		body.SetBullet(true);
		body.SetLinearVelocity({ initialVelocity.x, initialVelocity.y });
		missile.texture = TCM::Texture(missileTexture);
		missile.preserveTextureRatio = true;

		Globals::Components().renderingSetups().emplace_back([
			modelUniform = Uniforms::UniformControllerMat4f(), &body](Shaders::ProgramId program) mutable
			{
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
				modelUniform(Tools::GetModelMatrix(body));
				return nullptr;
			});

		missile.renderingSetup = Globals::Components().renderingSetups().size() - 1;

		missile.step = [&body, force, launchTime = Globals::Components().physics().simulationDuration]() mutable
		{
			body.ApplyForceToCenter({ glm::cos(body.GetAngle()) * force, glm::sin(body.GetAngle()) * force }, true);
		};

		auto& animationTexture = Globals::Components().animatedTextures().back();

		auto& decoration = EmplaceIdComponent(Globals::Components().temporaryFarMidgroundDecorations(), { Tools::CreateVerticesOfRectangle({ 0.0f, -0.5f }, { 0.5f, 0.5f }),
			TCM::AnimatedTexture(flameAnimatedTexture), Tools::CreateTexCoordOfRectangle() });

		Globals::Components().renderingSetups().emplace_back([&, modelUniform = Uniforms::UniformControllerMat4f(),
			thrustScale = 0.1f
			](Shaders::ProgramId program) mutable {
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
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
					particlesProgram.vp(Globals::Components().mvp().getVP());
					particlesProgram.texture1(explosionTexture);

					const float elapsed = Globals::Components().physics().simulationDuration - startTime;
					particlesProgram.color(glm::vec4(glm::vec3(glm::pow(1.0f - elapsed / (explosionDuration * 2.0f), 10.0f)), 1.0f));

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
			Globals::Components().renderingSetups().emplace_back([=, texturedProgram = Shaders::Programs::TexturedAccessor()
			](Shaders::ProgramId program) mutable {
				if (!texturedProgram.isValid()) texturedProgram = program;
				texturedProgram.vp(glm::translate(glm::scale(Globals::Components().mvp().getVP(), glm::vec3(glm::vec2(100.0f), 0.0f)),
					glm::vec3(-Globals::Components().camera().prevPosition * (0.002f + layer * 0.002f), 0.0f)));
				texturedProgram.color(fColor()* glm::vec4(1.0f, 1.0f, 1.0f, alphaPerLayer));

				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				return [texturedProgram]() mutable {
					texturedProgram.vp(Globals::Components().mvp().getVP());
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				};
			});

			Globals::Components().foregroundDecorations().emplace_back(Tools::CreateVerticesOfRectangle({ posXI, posYI }, glm::vec2(2.0f, 2.0f) + (layer * 0.2f)),
				TCM::Texture(fogTexture), Tools::CreateTexCoordOfRectangle(), Globals::Components().renderingSetups().size() - 1);

			Globals::Components().foregroundDecorations().back().resolutionMode = ResolutionMode::LowestLinearBlend1;
		}
	}

	void CreateJuliaBackground(Shaders::Programs::Julia& juliaShaders, std::function<glm::vec2()> juliaCOffset)
	{
		auto& background = Globals::Components().backgroundDecorations().emplace_back(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 10.0f, 10.0f }));
		background.customShadersProgram = juliaShaders.getProgramId();

		Globals::Components().renderingSetups().emplace_back([=, &juliaShaders
			](auto) mutable {
				juliaShaders.vp(glm::translate(glm::scale(glm::mat4(1.0f),
					glm::vec3((float)Globals::Components().screenInfo().windowSize.y / Globals::Components().screenInfo().windowSize.x, 1.0f, 1.0f) * 1.5f),
					glm::vec3(-Globals::Components().camera().prevPosition * 0.005f, 0.0f)));
				juliaShaders.juliaCOffset(juliaCOffset());
				juliaShaders.minColor({ 0.0f, 0.0f, 0.0f, 1.0f });
				juliaShaders.maxColor({ 0, 0.1f, 0.2f, 1.0f });
				return nullptr;
			});

		background.renderingSetup = Globals::Components().renderingSetups().size() - 1;
		background.resolutionMode = ResolutionMode::LowerLinearBlend1;
	}
}
