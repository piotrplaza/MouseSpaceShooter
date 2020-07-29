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
#include <ogl/shaders/julia.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/utility.hpp>

namespace Levels
{
	class Playground::Impl
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
		}

		void setBackground() const
		{
			using namespace Globals::Components;

			auto& background = backgroundDecorations.emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 10.0f, 10.0f }));
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
				vpUniform.setValue(glm::translate(glm::scale(glm::mat4(1.0f),
					glm::vec3((float)screenInfo.windowSize.y / screenInfo.windowSize.x, 1.0f, 1.0f) * 1.5f),
					glm::vec3(-camera.prevPosition * 0.005f, 0.0f)));
				juliaCOffsetUniform.setValue(player1->getCenter() * 0.00001f);
				minColorUniform.setValue({ 0.0f, 0.0f, 0.0f, 1.0f });
				maxColorUniform.setValue({ 0, 0.1f, 0.2f, 1.0f });

				return nullptr;
			};
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

					return nullptr;
				};

				for (int i = 0; i < 2; ++i)
				{
					auto& player1Thrust = player1Thrusts[i];

					backgroundDecorations.emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, -0.45f }, { 0.5f, 0.5f }), flameAnimation1Texture);
					backgroundDecorations.back().renderingSetup = [&, i,
						modelUniform = Uniforms::UniformControllerMat4f(),
						thrustScale = 1.0f
					](Shaders::ProgramId program) mutable {
						if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
						modelUniform.setValue(glm::scale(glm::rotate(glm::translate(Tools::GetModelMatrix(*player1->body),
							{ -0.9f, i == 0 ? -0.42f : 0.42f, 0.0f }),
							-glm::half_pi<float>() + (i == 0 ? 0.1f : -0.1f), { 0.0f, 0.0f, 1.0f }),
							{ std::min(thrustScale * 0.5f, 0.7f), thrustScale, 1.0f }));

						if (player1->throttling) thrustScale = std::min(thrustScale * 1.08f, 5.0f);
						else thrustScale = 1.0f + (thrustScale - 1.0f) * 0.95f;

						glBlendFunc(GL_ONE, GL_ONE);

						return []() { glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); };
					};

					backgroundDecorations.back().animationController.reset(new Tools::TextureAnimationController(
						{ 500, 498 }, { 2, 0 }, { 61, 120 }, { 8, 4 }, { 62.5f, 124.9f }, 0.02f, 0,
						AnimationLayout::Horizontal, AnimationPlayback::Backward, AnimationPolicy::Repeat,
						{ 0.0f, -0.45f }, { 1.0f, 1.0f }));
					player1Thrust = backgroundDecorations.back().animationController.get();
					player1Thrust->start();
				}
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
				textureTranslateUniform = Uniforms::UniformController2f()
			](Shaders::ProgramId program) mutable {
				if (!textureTranslateUniform.isValid()) textureTranslateUniform = Uniforms::UniformController2f(program, "textureTranslate");
				const float simulationTime = Globals::Components::physics.simulationTime;
				textureTranslateUniform.setValue({ glm::cos(simulationTime * 0.1f), glm::sin(simulationTime * 0.1f) });

				return nullptr;
			};

			for (const float pos : {-30.0f, 30.0f})
			{
				dynamicWalls.emplace_back(Tools::CreateCircleBody({ 0.0f, pos }, 5.0f, b2_dynamicBody, 0.01f), woodTexture);
				dynamicWalls.emplace_back(Tools::CreateCircleBody({ pos, 0.0f }, 10.0f, b2_dynamicBody, 0.01f), woodTexture);
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
					colorUniform = Uniforms::UniformController4f(),
					modelUniform = Uniforms::UniformControllerMat4f(),
					wallId = dynamicWalls.size() - 1
				](Shaders::ProgramId program) mutable {
					if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
					if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
					colorUniform.setValue({ 1.0f, 1.0f, 1.0f,
						(glm::sin(Globals::Components::physics.simulationTime * glm::two_pi<float>()) + 1.0f) / 2.0f + 0.5f });
					modelUniform.setValue(dynamicWalls[wallId].getModelMatrix());

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

			foregroundDecorations.emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 2.0f, 2.0f }), fogTexture);
			foregroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();
			foregroundDecorations.back().renderingSetup = [&,
				vpUniform = Uniforms::UniformControllerMat4f(),
				colorUniform = Uniforms::UniformController4f()
			](Shaders::ProgramId program) mutable {
				if (!vpUniform.isValid()) vpUniform = Uniforms::UniformControllerMat4f(program, "vp");
				if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
				vpUniform.setValue(glm::translate(glm::scale(glm::mat4(1.0f),
					glm::vec3((float)screenInfo.windowSize.y / screenInfo.windowSize.x, 1.0f, 1.0f) * 1.5f),
					glm::vec3(-camera.prevPosition * 0.02f, 0.0f)));
				colorUniform.setValue({1.0f, 1.0f, 1.0f, 0.2f});

				return nullptr;
			};
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
				return players.front().getCenter() + players.front().getVelocity() * 0.3f;
			};
		}

		void step()
		{
			for (auto& player1Thrust : player1Thrusts)
			{
				if (!player1Thrust) continue;
				if (Globals::Components::mouseState.xmb1) player1Thrust->pause();
				else player1Thrust->resume();
				if (Globals::Components::mouseState.xmb2) player1Thrust->start();

				player1Thrust->setTimeScale(1.0f + Globals::Components::mouseState.wheel / 10.0f);
			}
		}

	private:
		Shaders::Programs::Julia juliaShaders;

		unsigned rocketPlaneTexture = 0;
		unsigned spaceRockTexture = 0;
		unsigned woodTexture = 0;
		unsigned orbTexture = 0;
		unsigned weedTexture = 0;
		unsigned roseTexture = 0;
		unsigned fogTexture = 0;
		unsigned flameAnimation1Texture = 0;

		Components::Player* player1 = nullptr;
		Tools::TextureAnimationController* player1Thrusts[2] = {};
	};

	Playground::Playground(): impl(std::make_unique<Impl>())
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

	Playground::~Playground() = default;

	void Playground::step()
	{
		impl->step();
	}
}
