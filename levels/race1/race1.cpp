#include "race1.hpp"

#include <algorithm>

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
#include <components/mvp.hpp>

#include <ogl/uniformControllers.hpp>
#include <ogl/shaders/julia.hpp>
#include <ogl/shaders/texturedColorThreshold.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/renderingHelpers.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>

namespace Levels
{
	class Race1::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			using namespace Globals::Components;

			graphicsSettings.defaultColor = { 0.7f, 0.7f, 0.7f, 1.0f };
		}

		void loadTextures()
		{
			using namespace Globals::Components;

			rocketPlaneTexture = texturesDef.size();
			texturesDef.emplace_back("textures/rocket plane.png");
			texturesDef.back().translate = glm::vec2(-0.35f, -0.5f);
			texturesDef.back().scale = glm::vec2(1.7f);

			spaceRockTexture = texturesDef.size();
			texturesDef.emplace_back("textures/space rock.jpg", GL_MIRRORED_REPEAT);
			texturesDef.back().translate = glm::vec2(-0.35f, -0.5f);
			texturesDef.back().scale = glm::vec2(20.0f);

			woodTexture = texturesDef.size();
			texturesDef.emplace_back("textures/wood.jpg", GL_MIRRORED_REPEAT);
			texturesDef.back().translate = glm::vec2(-0.35f, -0.5f);
			texturesDef.back().scale = glm::vec2(16.0f);

			orbTexture = texturesDef.size();
			texturesDef.emplace_back("textures/orb.png");
			texturesDef.back().translate = glm::vec2(-0.5f);
			texturesDef.back().scale = glm::vec2(4.0f);

			weedTexture = texturesDef.size();
			texturesDef.emplace_back("textures/weed.png");
			texturesDef.back().minFilter = GL_LINEAR_MIPMAP_NEAREST;

			roseTexture = texturesDef.size();
			texturesDef.emplace_back("textures/rose.png");
			texturesDef.back().minFilter = GL_LINEAR_MIPMAP_NEAREST;

			fogTexture = texturesDef.size();
			texturesDef.emplace_back("textures/fog.png");

			flameAnimation1Texture = texturesDef.size();
			texturesDef.emplace_back("textures/flame animation 1.jpg");
			texturesDef.back().minFilter = GL_LINEAR;
		}

		void createBackground()
		{
			using namespace Globals::Components;

			Tools::CreateJuliaBackground(juliaShaders, []() { return players[0].getCenter() * 0.0001f; });
		}

		void createForeground() const
		{
			Tools::CreateFogForeground(2, 0.02f, fogTexture);
		}

		void createPlayers()
		{
			player1Handler = Tools::CreatePlayerPlane(rocketPlaneTexture, flameAnimation1Texture);
		}

		void createDynamicWalls()
		{
			using namespace Globals::Components;

			auto& wall1 = *dynamicWalls.emplace_back(
				Tools::CreateBoxBody({ 5.0f, -5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f), woodTexture).body;
			auto& wall2 = *dynamicWalls.emplace_back(
				Tools::CreateBoxBody({ 5.0f, 5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f), woodTexture).body;
			wall1.GetFixtureList()->SetRestitution(0.5f);
			wall2.GetFixtureList()->SetRestitution(0.5f);
			Tools::PinBodies(wall1, wall2, { 5.0f, 0.0f });
			dynamicWalls.back().renderingSetup = Tools::MakeUniqueRenderingSetup([
				textureTranslateUniform = Uniforms::UniformController2f()
			](Shaders::ProgramId program) mutable {
				if (!textureTranslateUniform.isValid()) textureTranslateUniform = Uniforms::UniformController2f(program, "textureTranslate");
				const float simulationDuration = Globals::Components::physics.simulationDuration;
				textureTranslateUniform.setValue({ glm::cos(simulationDuration * 0.1f), glm::sin(simulationDuration * 0.1f) });
				return nullptr;
			});

			for (const float pos : {-30.0f, 30.0f})
			{
				dynamicWalls.emplace_back(Tools::CreateCircleBody({ 0.0f, pos }, 5.0f, b2_dynamicBody, 0.01f), woodTexture,
					Tools::MakeUniqueRenderingSetup([this](auto)
					{
						Tools::MVPInitialization(texturedColorThresholdShaders);
						Tools::StaticTexturedRenderInitialization(texturedColorThresholdShaders, woodTexture, true);
						const float simulationDuration = Globals::Components::physics.simulationDuration;
						texturedColorThresholdShaders.invisibleColorUniform.setValue({ 1.0f, 1.0f, 1.0f });
						texturedColorThresholdShaders.invisibleColorThresholdUniform.setValue((-glm::cos(simulationDuration * 0.5f) + 1.0f) * 0.5f);
						return nullptr;
					}),
					texturedColorThresholdShaders.getProgramId());
				dynamicWalls.emplace_back(Tools::CreateCircleBody({ pos, 0.0f }, 10.0f, b2_dynamicBody, 0.01f));
				dynamicWalls.back().renderingSetup = Tools::MakeUniqueRenderingSetup([
					colorUniform = Uniforms::UniformController4f()
				](Shaders::ProgramId program) mutable {
					if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
					colorUniform.setValue({ 1.0f, 1.0f, 1.0f, 0.0f });
					return nullptr;
				});
				nearMidgroundDecorations.emplace_back(Tools::CreatePositionsOfFunctionalRectangles({ 1.0f, 1.0f },
					[](float input) { return glm::vec2(glm::cos(input * 100.0f) * input * 10.0f, glm::sin(input * 100.0f) * input * 10.0f); },
					[](float input) { return glm::vec2(input + 0.3f, input + 0.3f); },
					[](float input) { return input * 600.0f; },
					[value = 0.0f]() mutable->std::optional<float> {
					if (value > 1.0f) return std::nullopt;
					float result = value;
					value += 0.002f;
					return result;
				}
				), roseTexture);
				nearMidgroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();
				nearMidgroundDecorations.back().renderingSetup = Tools::MakeUniqueRenderingSetup([
					texturedProgramAccessor = std::optional<Shaders::Programs::TexturedAccessor>(),
						wallId = dynamicWalls.size() - 1
				](Shaders::ProgramId program) mutable {
					if (!texturedProgramAccessor) texturedProgramAccessor.emplace(program);
					texturedProgramAccessor->colorUniform.setValue({ 1.0f, 1.0f, 1.0f,
						(glm::sin(Globals::Components::physics.simulationDuration * glm::two_pi<float>()) + 1.0f) / 2.0f + 0.5f });
					texturedProgramAccessor->modelUniform.setValue(dynamicWalls[wallId].getModelMatrix());
					return nullptr;
				});
			}
		}

		void createStaticWalls() const
		{
			using namespace Globals::Components;

			const float levelHSize = 50.0f;
			const float bordersHGauge = 50.0f;

			staticWalls.emplace_back(Tools::CreateBoxBody({ -levelHSize - bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize + bordersHGauge * 2 }), spaceRockTexture);
			nearMidgroundDecorations.emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.0f, 1.0f }, { { -levelHSize, -levelHSize }, { levelHSize, -levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), weedTexture);
			nearMidgroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();

			staticWalls.emplace_back(Tools::CreateBoxBody({ levelHSize + bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize + bordersHGauge * 2 }), spaceRockTexture);
			nearMidgroundDecorations.emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.0f, 1.0f }, { { -levelHSize, levelHSize }, { levelHSize, levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), weedTexture);
			nearMidgroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();

			staticWalls.emplace_back(Tools::CreateBoxBody({ 0.0f, -levelHSize - bordersHGauge },
				{ levelHSize + bordersHGauge * 2, bordersHGauge }), spaceRockTexture);
			nearMidgroundDecorations.emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.0f, 1.0f }, { { -levelHSize, -levelHSize }, { -levelHSize, levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), weedTexture);
			nearMidgroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();

			staticWalls.emplace_back(Tools::CreateBoxBody({ 0.0f, levelHSize + bordersHGauge },
				{ levelHSize + bordersHGauge * 2, bordersHGauge }), spaceRockTexture);
			nearMidgroundDecorations.emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.0f, 1.0f }, { { levelHSize, -levelHSize }, { levelHSize, levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), weedTexture);
			nearMidgroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();
		}

		void createGrapples() const
		{
			using namespace Globals::Components;

			grapples.emplace_back(Tools::CreateCircleBody({ 0.0f, 10.0f }, 1.0f), 15.0f, orbTexture);
			grapples.emplace_back(Tools::CreateCircleBody({ 0.0f, -10.0f }, 1.0f), 15.0f, orbTexture);
			grapples.back().renderingSetup = Tools::MakeUniqueRenderingSetup([
				colorUniform = Uniforms::UniformController4f()
			](Shaders::ProgramId program) mutable {
				if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
				colorUniform.setValue({ 1.0f, 1.0f, 1.0f,
					(glm::sin(Globals::Components::physics.simulationDuration / 3.0f * glm::two_pi<float>()) + 1.0f) / 2.0f });
				return nullptr;
			});
			grapples.emplace_back(Tools::CreateCircleBody({ -10.0f, -30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f), 30.0f,
				orbTexture);
			auto& grapple = grapples.emplace_back(Tools::CreateCircleBody({ -10.0f, 30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f), 30.0f);

			farMidgroundDecorations.emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 1.8f, 1.8f }), roseTexture);
			farMidgroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();
			farMidgroundDecorations.back().renderingSetup = Tools::MakeUniqueRenderingSetup([&,
				modelUniform = Uniforms::UniformControllerMat4f()
			](Shaders::ProgramId program) mutable {
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
				modelUniform.setValue(grapple.getModelMatrix());
				return nullptr;
			});
		}

		void setCamera() const
		{
			using namespace Globals::Components;

			const auto& player = players[player1Handler.playerId];

			camera.targetProjectionHSizeF = [&]() {
				camera.projectionTransitionFactor = physics.frameDuration * 6;
				return 15.0f + glm::length(player.getVelocity()) * 0.2f;
			};
			camera.targetPositionF = [&]() {
				camera.positionTransitionFactor = physics.frameDuration * 6;
				return player.getCenter() + player.getVelocity() * 0.3f;
			};
		}

		void step()
		{
			using namespace Globals::Components;

			for (size_t backThrustsBackgroundDecorationId : player1Handler.backThrustsIds)
			{
				assert(backThrustsBackgroundDecorationId < farMidgroundDecorations.size());
				auto& player1ThrustAnimationController = *farMidgroundDecorations[backThrustsBackgroundDecorationId].animationController;
				//player1ThrustAnimationController.setTimeScale(1.0f + Globals::Components::mouseState.wheel / 10.0f);
			}
		}

	private:
		Shaders::Programs::Julia juliaShaders;
		Shaders::Programs::TexturedColorThreshold texturedColorThresholdShaders;

		unsigned rocketPlaneTexture = 0;
		unsigned spaceRockTexture = 0;
		unsigned woodTexture = 0;
		unsigned orbTexture = 0;
		unsigned weedTexture = 0;
		unsigned roseTexture = 0;
		unsigned fogTexture = 0;
		unsigned flameAnimation1Texture = 0;

		Tools::PlayerPlaneHandler player1Handler;
	};

	Race1::Race1() : impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->createBackground();
		impl->createPlayers();
		impl->createDynamicWalls();
		impl->createStaticWalls();
		impl->createGrapples();
		impl->createForeground();
		impl->setCamera();
	}

	Race1::~Race1() = default;

	void Race1::step()
	{
		impl->step();
	}
}
