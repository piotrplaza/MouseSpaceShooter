#include "playground.hpp"

#include <globals.hpp>

#include <components/screenInfo.hpp>
#include <components/physics.hpp>
#include <components/textureDef.hpp>
#include <components/player.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/camera.hpp>
#include <components/decoration.hpp>
#include <components/graphicsSettings.hpp>
#include <components/mouseState.hpp>

#include <ogl/uniformControllers.hpp>

#include <shaders/julia.hpp>

#include <tools/graphicsHelpers.hpp>

namespace Levels
{
	class Playground::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			using namespace Globals::Components;

			graphicsSettings.basicLevelColor = { 0.7f, 0.7f, 0.7f, 1.0f };
			graphicsSettings.texturedLevelColor = { 0.7f, 0.7f, 0.7f, 1.0f };
		}

		void setTextures()
		{
			using namespace Globals::Components;

			rocketPlaneTexture = texturesDef.size();
			texturesDef.emplace_back("textures/rocket plane.png");
			texturesDef.back().translate = glm::vec2(-0.35f, -0.5f);
			texturesDef.back().scale = glm::vec2(1.7f);

			spaceRockTexture = texturesDef.size();
			texturesDef.emplace_back("textures/space rock.jpg", GL_MIRRORED_REPEAT);
			texturesDef.back().scale = glm::vec2(20.0f);

			woodTexture = texturesDef.size();
			texturesDef.emplace_back("textures/wood.jpg", GL_MIRRORED_REPEAT);
			texturesDef.back().scale = glm::vec2(16.0f);

			orbTexture = texturesDef.size();
			texturesDef.emplace_back("textures/orb.png");
			texturesDef.back().translate = glm::vec2(-0.5f);
			texturesDef.back().scale = glm::vec2(4.0f);

			flameAnimation1Texture = texturesDef.size();
			texturesDef.emplace_back("textures/flame animation 1.jpg");
		}

		void setPlayers()
		{
			using namespace Globals::Components;

			player1 = &players.emplace_back(Tools::CreateTrianglePlayerBody(2.0f, 0.2f), rocketPlaneTexture);
			player1->setPosition({ -10.0f, 0.0f });
			player1->renderingSetup = [
				colorUniform = Uniforms::UniformController4f()
			](Shaders::ProgramId program) mutable {
				if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
				const float fade = (glm::sin(Globals::Components::physics.simulationTime * 2.0f * glm::two_pi<float>()) + 1.0f) / 2.0f;
				colorUniform.setValue({ fade, 1.0f, fade, 1.0f });
			};

			foregroundDecorations.emplace_back(Tools::CreateRectanglePositions({ 0.0f, -0.5f }, { 0.5f, 0.5f }), flameAnimation1Texture);
			foregroundDecorations.back().renderingSetup = [&,
				modelUniform = Uniforms::UniformControllerMat4f(),
				thrustScale = 1.0f
			](Shaders::ProgramId program) mutable {
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
				modelUniform.setValue(glm::scale(glm::rotate(glm::translate(Tools::GetModelMatrix(*player1->body), {-0.95f, 0.0f, 0.0f}),
					-glm::half_pi<float>(), { 0.0f, 0.0f, 1.0f }), { std::min(thrustScale, 2.0f), thrustScale, 1.0f }));

				if (player1->throttling && thrustScale < 10.0f) thrustScale *= 1.08f;
				else thrustScale = 1.0f + (thrustScale - 1.0f) * 0.95f;

				glBlendFunc(GL_ONE, GL_ONE);
			};

			foregroundDecorations.back().animationController.reset(new Tools::TextureAnimationController(
				{ 500, 498 }, { 2, 0 }, { 61, 120 }, { 8, 4 }, { 62.5f, 124.9f }, 0.02f, 0,
				AnimationLayout::Horizontal, AnimationPlayback::Backward, AnimationPolicy::Repeat,
				{ 0.0f, -0.5f }, { 1.0f, 1.0f }));
			player1Thrust = foregroundDecorations.back().animationController.get();
			player1Thrust->start();

			foregroundDecorations.emplace_back().renderingSetup = [](auto) {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			};
		}

		void setBackground() const
		{
			using namespace Globals::Components;

			auto& background = backgroundDecorations.emplace_back(Tools::CreateRectanglePositions({ 0.0f, 0.0f }, { 10.0f, 10.0f }));
			background.customShadersProgram = juliaShaders.program;
			background.renderingSetup = [&,
				vpUniform = Uniforms::UniformControllerMat4f(),
				juliaCOffsetUniform = Uniforms::UniformController2f(),
				minColorUniform = Uniforms::UniformController4f(),
				maxColorUniform = Uniforms::UniformController4f()
			](Shaders::ProgramId program) mutable {
				if (!vpUniform.isValid()) vpUniform = Uniforms::UniformControllerMat4f(program, "vp");
				if (!juliaCOffsetUniform.isValid()) juliaCOffsetUniform = Uniforms::UniformController2f(program, "juliaCOffset");
				if (!minColorUniform.isValid()) minColorUniform = Uniforms::UniformController4f(program, "minColor");
				if (!maxColorUniform.isValid()) maxColorUniform = Uniforms::UniformController4f(program, "maxColor");
				vpUniform.setValue(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-camera.prevPosition * 0.005f, 0.0f)),
					glm::vec3((float)screenInfo.windowSize.y / screenInfo.windowSize.x, 1.0f, 1.0f) * 1.5f));
				juliaCOffsetUniform.setValue(player1->getPosition() * 0.00001f);
				minColorUniform.setValue({ 0.0f, 0.0f, 0.0f, 1.0f });
				maxColorUniform.setValue({ 0, 0.1f, 0.2f, 1.0f });
			};
		}

		void setStaticWalls() const
		{
			using namespace Globals::Components;

			const float levelHSize = 50.0f;
			const float bordersHGauge = 50.0f;
			staticWalls.emplace_back(Tools::CreateBoxBody({ -levelHSize - bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize + bordersHGauge * 2 }), spaceRockTexture);
			staticWalls.emplace_back(Tools::CreateBoxBody({ levelHSize + bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize + bordersHGauge * 2 }), spaceRockTexture);
			staticWalls.emplace_back(Tools::CreateBoxBody({ 0.0f, -levelHSize - bordersHGauge },
				{ levelHSize + bordersHGauge * 2, bordersHGauge }), spaceRockTexture);
			staticWalls.emplace_back(Tools::CreateBoxBody({ 0.0f, levelHSize + bordersHGauge },
				{ levelHSize + bordersHGauge * 2, bordersHGauge }), spaceRockTexture);
			staticWalls.emplace_back(Tools::CreateCircleBody({ 10.0f, 0.0f }, 2.0f), spaceRockTexture);
			staticWalls.back().renderingSetup = [
				colorUniform = Uniforms::UniformController4f()
			](Shaders::ProgramId program) mutable {
				if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
				colorUniform.setValue({ 1.0f, 1.0f, 1.0f,
					(glm::sin(Globals::Components::physics.simulationTime * glm::two_pi<float>()) + 1.0f) / 2.0f });
			};
		}

		void setDynamicWalls() const
		{
			using namespace Globals::Components;

			auto& wall1 = *dynamicWalls.emplace_back(
				Tools::CreateBoxBody({ 5.0f, -5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f), woodTexture).body;
			auto& wall2 = *dynamicWalls.emplace_back(
				Tools::CreateBoxBody({ 5.0f, 5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f), woodTexture).body;
			wall1.GetFixtureList()->SetRestitution(0.5f);
			wall2.GetFixtureList()->SetRestitution(0.5f);
			Tools::PinBodies(wall1, wall2, { 5.0f, 0.0f });
			dynamicWalls.back().renderingSetup = [
				colorUniform = Uniforms::UniformController4f()
			](Shaders::ProgramId program) mutable {
				if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
				colorUniform.setValue({ 1.0f, 1.0f, 1.0f,
					(glm::sin(Globals::Components::physics.simulationTime / 2.0f * glm::two_pi<float>()) + 1.0f) / 2.0f });
			};
		}

		void setGrapples() const
		{
			using namespace Globals::Components;

			grapples.emplace_back(Tools::CreateCircleBody({ 0.0f, 10.0f }, 1.0f), 15.0f, orbTexture);
			grapples.emplace_back(Tools::CreateCircleBody({ 0.0f, -10.0f }, 1.0f), 15.0f, orbTexture);
			grapples.back().renderingSetup = [
				colorUniform = Uniforms::UniformController4f()
			](Shaders::ProgramId program) mutable {
				if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
				colorUniform.setValue({ 1.0f, 1.0f, 1.0f,
					(glm::sin(Globals::Components::physics.simulationTime / 3.0f * glm::two_pi<float>()) + 1.0f) / 2.0f });
			};
			grapples.emplace_back(Tools::CreateCircleBody({ -10.0f, -30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f), 30.0f,
				orbTexture);
			grapples.emplace_back(Tools::CreateCircleBody({ -10.0f, 30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f), 30.0f);
		}

		void setCamera() const
		{
			using namespace Globals::Components;

			camera.targetProjectionHSizeF = []() {
				camera.projectionTransitionFactor = 0.1f * physics.targetFrameTimeFactor;
				return 15.0f + glm::length(players.front().getVelocity()) * 0.2f;
			};
			camera.targetPositionF = []() {
				camera.positionTransitionFactor = 0.1f * physics.targetFrameTimeFactor;
				return players.front().getPosition() + players.front().getVelocity() * 0.3f;
			};
		}

		void step()
		{
			if (Globals::Components::mouseState.xmb1) player1Thrust->pause();
			else player1Thrust->resume();
			if (Globals::Components::mouseState.xmb2) player1Thrust->start();

			player1Thrust->setTimeScale(1.0f + Globals::Components::mouseState.wheel / 10.0f);
		}

	private:
		Shaders::Programs::Julia juliaShaders;

		unsigned rocketPlaneTexture = 0;
		unsigned spaceRockTexture = 0;
		unsigned woodTexture = 0;
		unsigned orbTexture = 0;
		unsigned flameAnimation1Texture = 0;

		Components::Player* player1 = nullptr;
		Tools::TextureAnimationController* player1Thrust = nullptr;
	};

	Playground::Playground(): impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->setTextures();
		impl->setPlayers();
		impl->setBackground();
		impl->setStaticWalls();
		impl->setDynamicWalls();
		impl->setGrapples();
		impl->setCamera();
	}

	Playground::~Playground() = default;

	void Playground::step()
	{
		impl->step();
	}
}
