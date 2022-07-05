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
#include <components/graphicsSettings.hpp>

#include <components/typeComponentMappers.hpp>

#include <globals/collisionBits.hpp>

#include <ogl/uniforms.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/shaders/julia.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/utility.hpp>

#include <cassert>

namespace Tools
{
	PlaneHandler CreatePlane(unsigned planeTexture, unsigned flameAnimatedTexture, glm::vec2 position, float angle)
	{
		PlaneHandler planeHandler;

		planeHandler.planeId = Globals::Components().planes().size();
		auto& plane = Globals::Components().planes().emplace_back(Tools::CreatePlaneBody(2.0f, 0.2f, 0.5f), TCM::Texture(planeTexture));
		SetCollisionFilteringBits(*plane.body, Globals::CollisionBits::planeBit, Globals::CollisionBits::all);
		plane.setPosition(position);
		plane.setRotation(angle);
		plane.preserveTextureRatio = true;

		Globals::Components().renderingSetups().emplace_back([
			colorUniform = Uniforms::Uniform4f()
			](Shaders::ProgramId program) mutable {
					if (!colorUniform.isValid()) colorUniform = Uniforms::Uniform4f(program, "color");
					const float fade = (glm::sin(Globals::Components().physics().simulationDuration * 2.0f * glm::two_pi<float>()) + 1.0f) / 2.0f;
					colorUniform({ fade, 1.0f, fade, 1.0f });
					return [=]() mutable { colorUniform(Globals::Components().graphicsSettings().defaultColor); };
				});

		plane.renderingSetup = Globals::Components().renderingSetups().size() - 1;

		for (int i = 0; i < 2; ++i)
		{
			auto& animationTexture = Globals::Components().animatedTextures().back();

			planeHandler.backThrustsIds[i] = Globals::Components().decorations().size();
			auto& decoration = Globals::Components().decorations().emplace_back(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 0.5f, 0.5f }),
				TCM::AnimatedTexture(flameAnimatedTexture));

			Globals::Components().renderingSetups().emplace_back([&, i, modelUniform = Uniforms::UniformMat4f(),
				thrustScale = 1.0f
				](Shaders::ProgramId program) mutable {
					if (!modelUniform.isValid()) modelUniform = Uniforms::UniformMat4f(program, "model");
					modelUniform(glm::scale(glm::rotate(glm::translate(Tools::GetModelMatrix(*plane.body),
						{ -1.0f - thrustScale * 0.25f, i == 0 ? -0.5f : 0.5f, 0.0f }),
						-glm::half_pi<float>() + (i == 0 ? 0.1f : -0.1f), { 0.0f, 0.0f, 1.0f }),
						{ std::min(thrustScale * 0.5f, 0.6f), thrustScale, 1.0f }));

					const float targetFrameDurationFactor = Globals::Components().physics().frameDuration * 6;
					if (plane.controls.throttling) thrustScale = std::min(thrustScale * (1.0f + targetFrameDurationFactor), 5.0f);
					else thrustScale = 1.0f + (thrustScale - 1.0f) * (1.0f - targetFrameDurationFactor);

					glBlendFunc(GL_SRC_ALPHA, GL_ONE);

					return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
				});

			decoration.renderingSetup = Globals::Components().renderingSetups().size() - 1;
			decoration.renderLayer = RenderLayer::FarMidground;
		}

		return planeHandler;
	}

	MissileHandler::MissileHandler() = default;

	MissileHandler::MissileHandler(ComponentId missileId, ComponentId backThrustId, glm::vec2 referenceVelocity):
		missileId(missileId),
		backThrustId(backThrustId),
		referenceVelocity(referenceVelocity)
	{
	}

	MissileHandler::~MissileHandler()
	{
		if (!valid) return;

		auto missileIt = Globals::Components().missiles().find(missileId);
		assert(missileIt != Globals::Components().missiles().end());
		missileIt->second.state = ComponentState::Outdated;
		auto thrustIt = Globals::Components().dynamicDecorations().find(backThrustId);
		assert(thrustIt != Globals::Components().dynamicDecorations().end());
		thrustIt->second.state = ComponentState::Outdated;
	}

	MissileHandler::MissileHandler(MissileHandler&& other) noexcept:
		missileId(other.missileId),
		backThrustId(other.backThrustId),
		referenceVelocity(other.referenceVelocity)
	{
		other.valid = false;
	}

	MissileHandler& MissileHandler::operator=(MissileHandler&& other) noexcept
	{
		missileId = other.missileId;
		backThrustId = other.backThrustId;
		referenceVelocity = other.referenceVelocity;
		other.valid = false;
		return *this;
	}

	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 referenceVelocity,
		glm::vec2 initialVelocity, unsigned missileTexture, unsigned flameAnimatedTexture)
	{
		auto& missile = EmplaceDynamicComponent(Globals::Components().missiles(), { Tools::CreateBoxBody(startPosition, { 0.5f, 0.2f }, startAngle, b2_dynamicBody, 0.2f) });
		auto& body = *missile.body;
		SetCollisionFilteringBits(body, Globals::CollisionBits::missileBit, Globals::CollisionBits::all - Globals::CollisionBits::missileBit - Globals::CollisionBits::planeBit);
		body.SetBullet(true);
		body.SetLinearVelocity(ToVec2<b2Vec2>(referenceVelocity + initialVelocity));
		missile.texture = TCM::Texture(missileTexture);
		missile.preserveTextureRatio = true;

		Globals::Components().renderingSetups().emplace_back([
			modelUniform = Uniforms::UniformMat4f(), &body](Shaders::ProgramId program) mutable
			{
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformMat4f(program, "model");
				modelUniform(Tools::GetModelMatrix(body));
				return nullptr;
			});

		missile.renderingSetup = Globals::Components().renderingSetups().size() - 1;
		missile.renderLayer = RenderLayer::FarMidground;

		missile.step = [&body, force, launchTime = Globals::Components().physics().simulationDuration]() mutable
		{
			body.ApplyForceToCenter({ glm::cos(body.GetAngle()) * force, glm::sin(body.GetAngle()) * force }, true);
		};

		auto& animationTexture = Globals::Components().animatedTextures().back();

		auto& decoration = EmplaceDynamicComponent(Globals::Components().dynamicDecorations(), { Tools::CreateVerticesOfRectangle({ 0.0f, -0.5f }, { 0.5f, 0.5f }),
			TCM::AnimatedTexture(flameAnimatedTexture), Tools::CreateTexCoordOfRectangle() });

		Globals::Components().renderingSetups().emplace_back([&, modelUniform = Uniforms::UniformMat4f(),
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

		return { missile.getComponentId(), decoration.getComponentId(), referenceVelocity };
	}

	void CreateExplosion(Shaders::Programs::ParticlesAccessor particlesProgram, glm::vec2 center, unsigned explosionTexture,
		float explosionDuration, int numOfParticles, int particlesPerDecoration, ResolutionMode resolutionMode)
	{
		Globals::Components().deferredActions().emplace_back([=](float) {
			auto& shockwave = EmplaceDynamicComponent(Globals::Components().shockwaves(), { center, numOfParticles });
			auto& explosionDecoration = EmplaceDynamicComponent(Globals::Components().dynamicDecorations(), {});
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

			Globals::Components().deferredActions().emplace_back([=, startTime = Globals::Components().physics().simulationDuration, &shockwave, &explosionDecoration](float) {
				const float elapsed = Globals::Components().physics().simulationDuration - startTime;
				const float scale = 1.0f + elapsed * 20.0f;

				if (elapsed > explosionDuration)
				{
					shockwave.state = ComponentState::Outdated;
					explosionDecoration.state = ComponentState::Outdated;
					return false;
				}

				explosionDecoration.vertices.clear();
				explosionDecoration.vertices.emplace_back(shockwave.center, scale);
				for (size_t i = 0; i < shockwave.particles.size(); ++i)
				{
					if (i % particlesPerDecoration != 0) continue;
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

			Globals::Components().decorations().emplace_back(Tools::CreateVerticesOfRectangle({ posXI, posYI }, glm::vec2(2.0f, 2.0f) + (layer * 0.2f)),
				TCM::Texture(fogTexture), Tools::CreateTexCoordOfRectangle(), Globals::Components().renderingSetups().size() - 1).renderLayer = RenderLayer::Foreground;
			Globals::Components().decorations().back().resolutionMode = ResolutionMode::LowestLinearBlend1;
		}
	}

	void CreateJuliaBackground(Shaders::Programs::Julia& juliaShaders, std::function<glm::vec2()> juliaCOffset)
	{
		auto& background = Globals::Components().decorations().emplace_back(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 10.0f, 10.0f }));
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
		background.renderLayer = RenderLayer::Background;
		background.resolutionMode = ResolutionMode::LowerLinearBlend1;
	}
}
