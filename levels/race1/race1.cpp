#include "race1.hpp"

#include <components/screenInfo.hpp>
#include <components/physics.hpp>
#include <components/renderingSetup.hpp>
#include <components/player.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/camera.hpp>
#include <components/decoration.hpp>
#include <components/graphicsSettings.hpp>
#include <components/mouseState.hpp>
#include <components/mvp.hpp>
#include <components/animatedTexture.hpp>

#include <ogl/uniformControllers.hpp>
#include <ogl/shaders/julia.hpp>
#include <ogl/shaders/texturedColorThreshold.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/renderingHelpers.hpp>

#include <globals/components.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>

#include <algorithm>

namespace Levels
{
	class Race1::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			Globals::Components().graphicsSettings().defaultColor = { 0.7f, 0.7f, 0.7f, 1.0f };
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().textures();

			rocketPlaneTexture = textures.size();
			textures.emplace_back("textures/rocket plane.png");
			textures.back().translate = glm::vec2(-0.35f, -0.5f);
			textures.back().scale = glm::vec2(1.7f);

			spaceRockTexture = textures.size();
			textures.emplace_back("textures/space rock.jpg", GL_MIRRORED_REPEAT);
			textures.back().translate = glm::vec2(-0.35f, -0.5f);
			textures.back().scale = glm::vec2(20.0f);

			woodTexture = textures.size();
			textures.emplace_back("textures/wood.jpg", GL_MIRRORED_REPEAT);
			textures.back().translate = glm::vec2(-0.35f, -0.5f);
			textures.back().scale = glm::vec2(16.0f);

			orbTexture = textures.size();
			textures.emplace_back("textures/orb.png");
			textures.back().translate = glm::vec2(-0.5f);
			textures.back().scale = glm::vec2(4.0f);

			weedTexture = textures.size();
			textures.emplace_back("textures/weed.png");
			textures.back().minFilter = GL_LINEAR_MIPMAP_NEAREST;

			roseTexture = textures.size();
			textures.emplace_back("textures/rose.png");
			textures.back().minFilter = GL_LINEAR_MIPMAP_NEAREST;

			fogTexture = textures.size();
			textures.emplace_back("textures/fog.png");

			flame1AnimationTexture = textures.size();
			textures.emplace_back("textures/flame animation 1.jpg");
			textures.back().minFilter = GL_LINEAR;
		}

		void setAnimations()
		{
			flame1AnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().push_back(Components::AnimatedTexture(
				flame1AnimationTexture, { 500, 498 }, { 2, 0 }, { 61, 120 }, { 8, 4 }, { 62.5f, 124.9f }, 0.02f, 0,
				AnimationLayout::Horizontal, AnimationPlayback::Backward, AnimationPolicy::Repeat,
				{ 0.0f, -0.45f }, { 1.0f, 1.0f }));
		}

		void createBackground()
		{
			Tools::CreateJuliaBackground(juliaShaders, []() { return Globals::Components().players()[1].getCenter() * 0.0001f; });
		}

		void createForeground() const
		{
			Tools::CreateFogForeground(2, 0.02f, fogTexture);
		}

		void createPlayers()
		{
			player1Handler = Tools::CreatePlayerPlane(rocketPlaneTexture, flame1AnimatedTexture);
		}

		void createDynamicWalls()
		{
			auto& wall1 = *Globals::Components().dynamicWalls().emplace_back(
				Tools::CreateBoxBody({ 5.0f, -5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f), TCM::Texture(woodTexture)).body;
			auto& wall2 = *Globals::Components().dynamicWalls().emplace_back(
				Tools::CreateBoxBody({ 5.0f, 5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f), TCM::Texture(woodTexture)).body;
			wall1.GetFixtureList()->SetRestitution(0.5f);
			wall2.GetFixtureList()->SetRestitution(0.5f);
			Tools::PinBodies(wall1, wall2, { 5.0f, 0.0f });

			Globals::Components().renderingSetups().emplace_back([
				textureTranslateUniform = Uniforms::UniformController2fv<5>()
				](Shaders::ProgramId program) mutable {
					if (!textureTranslateUniform.isValid()) textureTranslateUniform = Uniforms::UniformController2fv<5>(program, "texturesTranslate");
					const float simulationDuration = Globals::Components().physics().simulationDuration;
					textureTranslateUniform(0, { glm::cos(simulationDuration * 0.1f), glm::sin(simulationDuration * 0.1f) });
					return nullptr;
				});
			Globals::Components().dynamicWalls().back().renderingSetup = Globals::Components().renderingSetups().size() - 1;

			for (const float pos : {-30.0f, 30.0f})
			{
				Globals::Components().renderingSetups().emplace_back([this](auto) {
					Tools::MVPInitialization(texturedColorThresholdShaders);
					Tools::StaticTexturedRenderInitialization(texturedColorThresholdShaders, woodTexture, true);
					const float simulationDuration = Globals::Components().physics().simulationDuration;
					texturedColorThresholdShaders.invisibleColorUniform({ 1.0f, 1.0f, 1.0f });
					texturedColorThresholdShaders.invisibleColorThresholdUniform((-glm::cos(simulationDuration * 0.5f) + 1.0f) * 0.5f);
					return nullptr;
					});
				Globals::Components().dynamicWalls().emplace_back(Tools::CreateCircleBody({ 0.0f, pos }, 5.0f, b2_dynamicBody, 0.01f), TCM::Texture(woodTexture),
					Globals::Components().renderingSetups().size() - 1,
					texturedColorThresholdShaders.getProgramId());
				Globals::Components().dynamicWalls().emplace_back(Tools::CreateCircleBody({ pos, 0.0f }, 10.0f, b2_dynamicBody, 0.01f));

				Globals::Components().renderingSetups().emplace_back([
					colorUniform = Uniforms::UniformController4f()
					](Shaders::ProgramId program) mutable {
						if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
						colorUniform({ 1.0f, 1.0f, 1.0f, 0.0f });
						return nullptr;
					});
				Globals::Components().dynamicWalls().back().renderingSetup = Globals::Components().renderingSetups().size() - 1;

				Globals::Components().nearMidgroundDecorations().emplace_back(Tools::CreatePositionsOfFunctionalRectangles({ 1.0f, 1.0f },
					[](float input) { return glm::vec2(glm::cos(input * 100.0f) * input * 10.0f, glm::sin(input * 100.0f) * input * 10.0f); },
					[](float input) { return glm::vec2(input + 0.3f, input + 0.3f); },
					[](float input) { return input * 600.0f; },
					[value = 0.0f]() mutable->std::optional<float> {
					if (value > 1.0f) return std::nullopt;
					float result = value;
					value += 0.002f;
					return result;
				}
				), TCM::Texture(roseTexture));

				Globals::Components().nearMidgroundDecorations().back().texCoord = Tools::CreateTexCoordOfRectangle();

				Globals::Components().renderingSetups().emplace_back([
					texturedProgramAccessor = std::optional<Shaders::Programs::TexturedAccessor>(),
					wallId = Globals::Components().dynamicWalls().size() - 1
					](Shaders::ProgramId program) mutable {
						if (!texturedProgramAccessor) texturedProgramAccessor.emplace(program);
						texturedProgramAccessor->colorUniform({ 1.0f, 1.0f, 1.0f,
							(glm::sin(Globals::Components().physics().simulationDuration * glm::two_pi<float>()) + 1.0f) / 2.0f + 0.5f });
						texturedProgramAccessor->modelUniform(Globals::Components().dynamicWalls()[wallId].getModelMatrix());
						return nullptr;
					});
				Globals::Components().nearMidgroundDecorations().back().renderingSetup = Globals::Components().renderingSetups().size() - 1;
			}
		}

		void createStaticWalls() const
		{
			const float levelHSize = 50.0f;
			const float bordersHGauge = 50.0f;

			Globals::Components().staticWalls().emplace_back(Tools::CreateBoxBody({ -levelHSize - bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize + bordersHGauge * 2 }), TCM::Texture(spaceRockTexture));
			Globals::Components().nearMidgroundDecorations().emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.0f, 1.0f }, { { -levelHSize, -levelHSize }, { levelHSize, -levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), TCM::Texture(weedTexture));
			Globals::Components().nearMidgroundDecorations().back().texCoord = Tools::CreateTexCoordOfRectangle();

			Globals::Components().staticWalls().emplace_back(Tools::CreateBoxBody({ levelHSize + bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize + bordersHGauge * 2 }), TCM::Texture(spaceRockTexture));
			Globals::Components().nearMidgroundDecorations().emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.0f, 1.0f }, { { -levelHSize, levelHSize }, { levelHSize, levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), TCM::Texture(weedTexture));
			Globals::Components().nearMidgroundDecorations().back().texCoord = Tools::CreateTexCoordOfRectangle();

			Globals::Components().staticWalls().emplace_back(Tools::CreateBoxBody({ 0.0f, -levelHSize - bordersHGauge },
				{ levelHSize + bordersHGauge * 2, bordersHGauge }), TCM::Texture(spaceRockTexture));
			Globals::Components().nearMidgroundDecorations().emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.0f, 1.0f }, { { -levelHSize, -levelHSize }, { -levelHSize, levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), TCM::Texture(weedTexture));
			Globals::Components().nearMidgroundDecorations().back().texCoord = Tools::CreateTexCoordOfRectangle();

			Globals::Components().staticWalls().emplace_back(Tools::CreateBoxBody({ 0.0f, levelHSize + bordersHGauge },
				{ levelHSize + bordersHGauge * 2, bordersHGauge }), TCM::Texture(spaceRockTexture));
			Globals::Components().nearMidgroundDecorations().emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.0f, 1.0f }, { { levelHSize, -levelHSize }, { levelHSize, levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), TCM::Texture(weedTexture));
			Globals::Components().nearMidgroundDecorations().back().texCoord = Tools::CreateTexCoordOfRectangle();
		}

		void createGrapples() const
		{
			Globals::Components().grapples().emplace_back(Tools::CreateCircleBody({ 0.0f, 10.0f }, 1.0f), 15.0f, TCM::Texture(orbTexture));
			Globals::Components().grapples().emplace_back(Tools::CreateCircleBody({ 0.0f, -10.0f }, 1.0f), 15.0f, TCM::Texture(orbTexture));

			Globals::Components().renderingSetups().emplace_back([
				colorUniform = Uniforms::UniformController4f()
				](Shaders::ProgramId program) mutable {
					if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
					colorUniform({ 1.0f, 1.0f, 1.0f,
						(glm::sin(Globals::Components().physics().simulationDuration / 3.0f * glm::two_pi<float>()) + 1.0f) / 2.0f });
					return nullptr;
				});
			Globals::Components().grapples().back().renderingSetup = Globals::Components().renderingSetups().size() - 1;

			Globals::Components().grapples().emplace_back(Tools::CreateCircleBody({ -10.0f, -30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f), 30.0f,
				TCM::Texture(orbTexture));
			auto& grapple = Globals::Components().grapples().emplace_back(Tools::CreateCircleBody({ -10.0f, 30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f), 30.0f);

			Globals::Components().farMidgroundDecorations().emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 1.8f, 1.8f }), TCM::Texture(roseTexture));
			Globals::Components().farMidgroundDecorations().back().texCoord = Tools::CreateTexCoordOfRectangle();

			Globals::Components().renderingSetups().emplace_back([&,
				modelUniform = Uniforms::UniformControllerMat4f()
				](Shaders::ProgramId program) mutable {
					if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
					modelUniform(grapple.getModelMatrix());
					return nullptr;
				});
			Globals::Components().farMidgroundDecorations().back().renderingSetup = Globals::Components().renderingSetups().size() - 1;
		}

		void setCamera() const
		{
			const auto& player = Globals::Components().players()[player1Handler.playerId];

			Globals::Components().camera().targetProjectionHSizeF = [&]() {
				Globals::Components().camera().projectionTransitionFactor = Globals::Components().physics().frameDuration * 6;
				return 15.0f + glm::length(player.getVelocity()) * 0.2f;
			};
			Globals::Components().camera().targetPositionF = [&]() {
				Globals::Components().camera().positionTransitionFactor = Globals::Components().physics().frameDuration * 6;
				return player.getCenter() + player.getVelocity() * 0.3f;
			};
		}

		void step()
		{
			for (size_t backThrustsBackgroundDecorationId : player1Handler.backThrustsIds)
			{
				assert(backThrustsBackgroundDecorationId < Globals::Components().farMidgroundDecorations().size());
				auto& player1ThrustAnimationTexture = Globals::Components().animatedTextures()[
					std::get<TCM::AnimatedTexture>(Globals::Components().farMidgroundDecorations()[backThrustsBackgroundDecorationId].texture).id];
				//player1ThrustAnimationTexture.setTimeScale(1.0f + Globals::Components::mouseState.wheel / 10.0f);
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
		unsigned flame1AnimationTexture = 0;

		unsigned flame1AnimatedTexture = 0;

		Tools::PlayerPlaneHandler player1Handler;
	};

	Race1::Race1():
		impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->setAnimations();
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
