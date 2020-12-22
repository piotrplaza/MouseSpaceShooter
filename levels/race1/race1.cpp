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

		void setTextures()
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

		void setBackground()
		{
			using namespace Globals::Components;

			auto& background = backgroundDecorations.emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 10.0f, 10.0f }));
			background.customShadersProgram = juliaShaders.program;
			background.renderingSetup = [this](auto) mutable {
				const auto& player = players[player1Handler.playerId];
				juliaShaders.vpUniform.setValue(glm::translate(glm::scale(glm::mat4(1.0f),
					glm::vec3((float)screenInfo.windowSize.y / screenInfo.windowSize.x, 1.0f, 1.0f) * 1.5f),
					glm::vec3(-camera.prevPosition * 0.005f, 0.0f)));
				juliaShaders.juliaCOffsetUniform.setValue(player.getCenter() * 0.00001f);
				juliaShaders.minColorUniform.setValue({ 0.0f, 0.0f, 0.0f, 1.0f });
				juliaShaders.maxColorUniform.setValue({ 0, 0.1f, 0.2f, 1.0f });

				return nullptr;
			};
		}

		void setPlayers()
		{
			player1Handler = Tools::CreatePlayerPlane(rocketPlaneTexture, flameAnimation1Texture);
		}

		void setDynamicWalls()
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
				textureTranslateUniform = Uniforms::UniformController2f()
			](Shaders::ProgramId program) mutable {
					if (!textureTranslateUniform.isValid()) textureTranslateUniform = Uniforms::UniformController2f(program, "textureTranslate");
					const float simulationTime = Globals::Components::physics.simulationTime;
					textureTranslateUniform.setValue({ glm::cos(simulationTime * 0.1f), glm::sin(simulationTime * 0.1f) });

					return nullptr;
				};

				for (const float pos : {-30.0f, 30.0f})
				{
					dynamicWalls.emplace_back(Tools::CreateCircleBody({ 0.0f, pos }, 5.0f, b2_dynamicBody, 0.01f), woodTexture,
						[this](auto)
						{
							Tools::MVPInitialization(texturedColorThresholdShaders);
							Tools::StaticTexturedRenderInitialization(texturedColorThresholdShaders, woodTexture, true);
							const float simulationTime = Globals::Components::physics.simulationTime;
							texturedColorThresholdShaders.invisibleColorUniform.setValue({ 1.0f, 1.0f, 1.0f });
							texturedColorThresholdShaders.invisibleColorThresholdUniform.setValue((-glm::cos(simulationTime * 0.5f) + 1.0f) * 0.5f);
							return nullptr;
						},
						texturedColorThresholdShaders.program);
					dynamicWalls.emplace_back(Tools::CreateCircleBody({ pos, 0.0f }, 10.0f, b2_dynamicBody, 0.01f));
					dynamicWalls.back().renderingSetup = [
						colorUniform = Uniforms::UniformController4f()
					](Shaders::ProgramId program) mutable {
							if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
							colorUniform.setValue({ 1.0f, 1.0f, 1.0f, 0.0f });

							return nullptr;
						};
						foregroundDecorations.emplace_back(Tools::CreatePositionsOfFunctionalRectangles({ 1.0f, 1.0f },
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
						foregroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();
						foregroundDecorations.back().renderingSetup = [
							texturedProgramAccessor = std::optional<Shaders::Programs::TexturedAccessor>(),
								wallId = dynamicWalls.size() - 1
						](Shaders::ProgramId program) mutable {
								if (!texturedProgramAccessor) texturedProgramAccessor.emplace(program);
								texturedProgramAccessor->colorUniform.setValue({ 1.0f, 1.0f, 1.0f,
									(glm::sin(Globals::Components::physics.simulationTime * glm::two_pi<float>()) + 1.0f) / 2.0f + 0.5f });
								texturedProgramAccessor->modelUniform.setValue(dynamicWalls[wallId].getModelMatrix());

								return nullptr;
							};
				}
		}

		void setStaticWalls() const
		{
			using namespace Globals::Components;

			const float levelHSize = 50.0f;
			const float bordersHGauge = 50.0f;

			staticWalls.emplace_back(Tools::CreateBoxBody({ -levelHSize - bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize + bordersHGauge * 2 }), spaceRockTexture);
			foregroundDecorations.emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.0f, 1.0f }, { { -levelHSize, -levelHSize }, { levelHSize, -levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), weedTexture);
			foregroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();

			staticWalls.emplace_back(Tools::CreateBoxBody({ levelHSize + bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize + bordersHGauge * 2 }), spaceRockTexture);
			foregroundDecorations.emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.0f, 1.0f }, { { -levelHSize, levelHSize }, { levelHSize, levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), weedTexture);
			foregroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();

			staticWalls.emplace_back(Tools::CreateBoxBody({ 0.0f, -levelHSize - bordersHGauge },
				{ levelHSize + bordersHGauge * 2, bordersHGauge }), spaceRockTexture);
			foregroundDecorations.emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.0f, 1.0f }, { { -levelHSize, -levelHSize }, { -levelHSize, levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), weedTexture);
			foregroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();

			staticWalls.emplace_back(Tools::CreateBoxBody({ 0.0f, levelHSize + bordersHGauge },
				{ levelHSize + bordersHGauge * 2, bordersHGauge }), spaceRockTexture);
			foregroundDecorations.emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.0f, 1.0f }, { { levelHSize, -levelHSize }, { levelHSize, levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), weedTexture);
			foregroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();
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

					return nullptr;
				};
				grapples.emplace_back(Tools::CreateCircleBody({ -10.0f, -30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f), 30.0f,
					orbTexture);
				auto& grapple = grapples.emplace_back(Tools::CreateCircleBody({ -10.0f, 30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f), 30.0f);

				midgroundDecorations.emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 1.8f, 1.8f }), roseTexture);
				midgroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();
				midgroundDecorations.back().renderingSetup = [&,
					modelUniform = Uniforms::UniformControllerMat4f()
				](Shaders::ProgramId program) mutable {
					if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
					modelUniform.setValue(grapple.getModelMatrix());

					return nullptr;
				};
		}

		void setForeground() const
		{
			using namespace Globals::Components;

			for (int layer = 0; layer < 2; ++layer)
				for (int posYI = -1; posYI <= 1; ++posYI)
					for (int posXI = -1; posXI <= 1; ++posXI)
					{
						foregroundDecorations.emplace_back(Tools::CreatePositionsOfRectangle({ posXI, posYI }, glm::vec2(2.0f, 2.0f) + (layer * 0.2f)), fogTexture);
						foregroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();
						foregroundDecorations.back().renderingSetup = [&,
							texturedProgramAccessor = std::optional<Shaders::Programs::TexturedAccessor>(),
							layer
						](Shaders::ProgramId program) mutable {
							if (!texturedProgramAccessor) texturedProgramAccessor.emplace(program);
							texturedProgramAccessor->vpUniform.setValue(glm::translate(glm::scale(glm::mat4(1.0f),
								glm::vec3((float)screenInfo.windowSize.y / screenInfo.windowSize.x, 1.0f, 1.0f) * 1.5f),
								glm::vec3(-camera.prevPosition * (0.02f + layer * 0.02f), 0.0f)));
							texturedProgramAccessor->colorUniform.setValue({ 1.0f, 1.0f, 1.0f, 0.02f });

							return nullptr;
						};
					}
		}

		void setCamera() const
		{
			using namespace Globals::Components;

			const auto& player = players[player1Handler.playerId];

			camera.targetProjectionHSizeF = [&]() {
				camera.projectionTransitionFactor = physics.frameTime * 6;
				return 15.0f + glm::length(player.getVelocity()) * 0.2f;
			};
			camera.targetPositionF = [&]() {
				camera.positionTransitionFactor = physics.frameTime * 6;
				return player.getCenter() + player.getVelocity() * 0.3f;
			};
		}

		void step()
		{
			using namespace Globals::Components;

			for (size_t backThrustsBackgroundDecorationId : player1Handler.backThrustsBackgroundDecorationIds)
			{
				assert(backThrustsBackgroundDecorationId < backgroundDecorations.size());
				auto& player1ThrustAnimationController = backgroundDecorations[backThrustsBackgroundDecorationId].animationController;
				player1ThrustAnimationController->setTimeScale(1.0f + Globals::Components::mouseState.wheel / 10.0f);
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
		impl->setTextures();
		impl->setBackground();
		impl->setPlayers();
		impl->setDynamicWalls();
		impl->setStaticWalls();
		impl->setGrapples();
		impl->setForeground();
		impl->setCamera();
	}

	Race1::~Race1() = default;

	void Race1::step()
	{
		impl->step();
	}
}
