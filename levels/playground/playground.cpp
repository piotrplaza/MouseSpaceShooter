#include "playground.hpp"

#include <components/screenInfo.hpp>
#include <components/physics.hpp>
#include <components/renderingSetup.hpp>
#include <components/plane.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/camera.hpp>
#include <components/decoration.hpp>
#include <components/graphicsSettings.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>
#include <components/mvp.hpp>
#include <components/missile.hpp>
#include <components/collisionFilter.hpp>
#include <components/collisionHandler.hpp>
#include <components/shockwave.hpp>
#include <components/functor.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/blendingTexture.hpp>
#include <components/animatedTexture.hpp>
#include <components/deferredAction.hpp>

#include <globals/shaders.hpp>
#include <globals/components.hpp>
#include <globals/systems.hpp>
#include <globals/collisionBits.hpp>

#include <ogl/uniforms.hpp>
#include <ogl/shaders/texturedColorThreshold.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/renderingHelpers.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>

#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <optional>

namespace
{
	constexpr bool gamepadForPlayer1 = false;
}

namespace Levels
{
	class Playground::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			Globals::Components().graphicsSettings().defaultColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			Globals::Components().mainFramebufferRenderer().renderer = Tools::Demo3DRotatedFullscreenRenderer(Globals::Shaders().textured());
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().textures();

			rocketPlaneTexture = textures.size();
			textures.emplace_back("textures/rocket plane.png");
			textures.back().translate = glm::vec2(0.4f, 0.0f);
			textures.back().scale = glm::vec2(1.6f, 1.8f);

			spaceRockTexture = textures.size();
			textures.emplace_back("textures/space rock.jpg", GL_MIRRORED_REPEAT);
			textures.back().scale = glm::vec2(5.0f);

			woodTexture = textures.size();
			textures.emplace_back("textures/wood.jpg", GL_MIRRORED_REPEAT);
			textures.back().scale = glm::vec2(5.0f);

			orbTexture = textures.size();
			textures.emplace_back("textures/orb.png");
			textures.back().scale = glm::vec2(2.0f);

			weedTexture = textures.size();
			textures.emplace_back("textures/weed.png");
			textures.back().minFilter = GL_LINEAR_MIPMAP_LINEAR;

			roseTexture = textures.size();
			textures.emplace_back("textures/rose.png");
			textures.back().minFilter = GL_LINEAR_MIPMAP_LINEAR;

			fogTexture = textures.size();
			textures.emplace_back("textures/fog.png");

			flameAnimationTexture = textures.size();
			textures.emplace_back("textures/flame animation 1.jpg");
			textures.back().minFilter = GL_LINEAR;

			missile1Texture = textures.size();
			textures.emplace_back("textures/missile1.png");
			textures.back().minFilter = GL_LINEAR;
			textures.back().translate = glm::vec2(0.4f, 0.0f);
			textures.back().scale = glm::vec2(0.3f, 0.4f);

			missile2Texture = textures.size();
			textures.emplace_back("textures/missile2.png");
			textures.back().minFilter = GL_LINEAR;
			textures.back().scale = glm::vec2(0.4f, 0.45f);

			explosionTexture = textures.size();
			textures.emplace_back("textures/explosion.png");

			foiledEggsTexture = textures.size();
			textures.emplace_back("textures/foiled eggs.jpg", GL_MIRRORED_REPEAT);
			textures.back().scale = glm::vec2(100.0f, 100.0f);

			fractalTexture = textures.size();
			textures.emplace_back("textures/fractal.jpg", GL_MIRRORED_REPEAT);
			textures.back().scale = glm::vec2(50.0f, 50.0f);

			mosaicTexture = textures.size();
			textures.emplace_back("textures/mosaic.jpg", GL_MIRRORED_REPEAT);
			textures.back().scale = glm::vec2(1.0f, 1.0f);

			ppTexture = textures.size();
			textures.emplace_back("textures/pp.png");

			skullTexture = textures.size();
			textures.emplace_back("textures/skull_rot.png");
			textures.back().translate = glm::vec2(0.02f, 0.21f);
			textures.back().scale = glm::vec2(0.46f, 0.44f);

			avatarTexture = textures.size();
			textures.emplace_back("textures/avatar_rot.png");
			textures.back().translate = glm::vec2(0.02f, 0.16f);
			textures.back().scale = glm::vec2(0.29f, 0.32f);
			textures.back().darkToTransparent = true;
		}

		void setAnimations()
		{
			for (unsigned& flameAnimatedTextureForPlayer: flameAnimatedTextureForPlayers)
			{
				flameAnimatedTextureForPlayer = Globals::Components().animatedTextures().size();
				Globals::Components().animatedTextures().push_back(Components::AnimatedTexture(
					flameAnimationTexture, { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
					AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal));
				Globals::Components().animatedTextures().back().start(true);
			}

			flameAnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().push_back(Globals::Components().animatedTextures().back());

			invertedFlameAnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().push_back(Globals::Components().animatedTextures().back());
			Globals::Components().animatedTextures().back().setAdditionalTransformation({ 0.0f, 0.0f }, glm::pi<float>());
		}

		void createBackground()
		{
			Tools::CreateJuliaBackground([this]() {
				const auto averageCenter = std::accumulate(playersHandlers.begin(), playersHandlers.end(),
					glm::vec2(0.0f), [](const auto& acc, const auto& currentHandler) {
						return acc + Globals::Components().planes()[currentHandler.playerId].getCenter();
					}) / (float)playersHandlers.size();
				return averageCenter * 0.0001f; });
		}

		void createForeground()
		{
			Tools::CreateFogForeground(2, 0.02f, fogTexture, [&, fogTargetAlphaFactor = 1.0f]() mutable {
				if (explosionFrame)
				{
					const float maxAlphaFactor = 2.0f;
					fogTargetAlphaFactor = fogTargetAlphaFactor < maxAlphaFactor
						? fogTargetAlphaFactor + Globals::Components().physics().frameDuration * 10.0f
						: maxAlphaFactor;
				}
				else
				{
					fogTargetAlphaFactor = fogTargetAlphaFactor > 1.0f
						? fogTargetAlphaFactor - Globals::Components().physics().frameDuration * 0.2f
						: 1.0f;
				}
				fogAlphaFactor += (fogTargetAlphaFactor - fogAlphaFactor) * 0.05f * Globals::Components().physics().frameDuration;
				return glm::vec4(1.0f, 1.0f, 1.0f, fogAlphaFactor);
				});
		}

		void createAdditionalDecorations() const
		{
			const auto blendingTexture = Globals::Components().blendingTextures().size();
			Globals::Components().blendingTextures().push_back({ { invertedFlameAnimatedTexture, ppTexture, skullTexture, avatarTexture }, true });

			for (int i = 0; i < 2; ++i)
			{
				glm::vec2 portraitCenter(i == 0 ? -40.0f : 40.0f, -40.0f);
				Globals::Components().renderingSetups().emplace_back([=,
					addBlendingColor = Uniforms::Uniform4f()
				](Shaders::ProgramId program) mutable {
					if (!addBlendingColor.isValid())
						addBlendingColor = Uniforms::Uniform4f(program, "addBlendingColor");

					const float skullOpacity = fogAlphaFactor - 1.0f;
					float minDistance = std::numeric_limits<float>::max();
					for (const auto& playerHandler : playersHandlers)
						minDistance = std::min(minDistance, glm::distance(Globals::Components().planes()[playerHandler.playerId].getCenter(), portraitCenter));
					const float avatarOpacity = glm::min(0.0f, minDistance / 3.0f - 5.0f);

					i == 0
						? addBlendingColor({ 1.0f, skullOpacity, avatarOpacity, 0.0f })
						: addBlendingColor({ 0.0f, 0.0f, 1.0f, 0.0f });

					return [=]() mutable {
						addBlendingColor({ 0.0f, 0.0f, 0.0f, 0.0f });
					};
				});

				Globals::Components().decorations().emplace_back(Tools::CreateVerticesOfRectangle(portraitCenter, { 10.0f, 10.0f }),
					TCM::BlendingTexture(blendingTexture), Tools::CreateTexCoordOfRectangle(), Globals::Components().renderingSetups().size() - 1,
					RenderLayer::NearMidground).preserveTextureRatio = true;
			}
		}

		void updateNumOfPlayers(bool init)
		{
			const auto& gamepads = Globals::Components().gamepads();
			auto& planes = Globals::Components().planes();
			std::vector<unsigned> activeGamepads;

			for (unsigned i = 0; i < gamepads.size(); ++i)
			{
				const auto& gamepad = gamepads[i];
				if (gamepad.enabled)
					activeGamepads.push_back(i);
			}

			const unsigned numOfPlayers = std::clamp(activeGamepads.size() + !gamepadForPlayer1, 1u, 4u);

			if (init)
			{
				const float gap = 5.0f;
				const float farPlayersDistance = gap * (numOfPlayers - 1);

				playersHandlers.reserve(numOfPlayers);
				unsigned activeGamepadId = 0;
				for (unsigned i = 0; i < numOfPlayers; ++i)
					playersHandlers.emplace_back(Tools::CreatePlane(rocketPlaneTexture, flameAnimatedTextureForPlayers[i], { -10.0f, -farPlayersDistance / 2.0f + gap * i }),
						i == 0 && !gamepadForPlayer1 || activeGamepads.empty() ? std::nullopt : std::optional(activeGamepads[activeGamepadId++]), 0.0f);
			}
			else
			{
				std::erase_if(playersHandlers, [&, i = 0](const auto& playerHandler) mutable {
					const bool erase = i++ != 0 && playerHandler.gamepadId && !gamepads[*playerHandler.gamepadId].enabled;
					if (erase)
					{
						planes[playerHandler.playerId].state = ComponentState::Outdated;
						return true;
					}
					return false;
					});

				for (auto activeGamepadId: activeGamepads)
				{
					auto it = std::find_if(playersHandlers.begin(), playersHandlers.end(), [&](const auto& playerHandler) {
							return playerHandler.gamepadId && playerHandler.gamepadId == activeGamepadId;
						});
					if (it == playersHandlers.end())
						if (gamepadForPlayer1 && !playersHandlers[0].gamepadId)
							playersHandlers[0].gamepadId = activeGamepadId;
						else
							playersHandlers.emplace_back(Tools::CreatePlane(rocketPlaneTexture, playersHandlers.size() - 1, { -10.0f, 0.0f }), activeGamepadId, 0.0f);
				}
			}
		}

		void launchingMissile(unsigned playerHandlerId, bool tryToLaunch)
		{
			auto& playerHandler = playersHandlers[playerHandlerId];
			if (tryToLaunch)
			{
				if (playerHandler.durationToLaunchMissile <= 0.0f)
				{
					launchMissile(playerHandler.playerId);
					playerHandler.durationToLaunchMissile = 0.1f;
				}
				else playerHandler.durationToLaunchMissile -= Globals::Components().physics().frameDuration;
			}
			else playerHandler.durationToLaunchMissile = 0.0f;
		}

		void launchMissile(unsigned playerId)
		{
			auto missileHandler = Tools::CreateMissile(Globals::Components().planes()[playerId].getCenter(),
				Globals::Components().planes()[playerId].getAngle(), 5.0f, {0.0f, 0.0f},
				Globals::Components().planes()[playerId].getVelocity(),
				missile2Texture, flameAnimatedTexture, playerId);
			missilesToHandlers.emplace(missileHandler.missileId, std::move(missileHandler));
		}

		void createMovableWalls()
		{
			Globals::Components().renderingSetups().emplace_back([
				texturesCustomTransformUniform = Uniforms::UniformMat4f()
			](Shaders::ProgramId program) mutable {
					if (!texturesCustomTransformUniform.isValid()) texturesCustomTransformUniform = Uniforms::UniformMat4f(program, "texturesCustomTransform");
					const float simulationDuration = Globals::Components().physics().simulationDuration;
					texturesCustomTransformUniform(Tools::TextureTransform(glm::vec2(glm::cos(simulationDuration), glm::sin(simulationDuration)) * 0.1f ));
					return [=]() mutable { texturesCustomTransformUniform(glm::mat4(1.0f)); };
				});

			{
				auto setRenderingSetupAndSubsequence = [&]()
				{
					Globals::Components().walls().back().subsequence.emplace_back(Tools::CreateVerticesOfLineOfRectangles({ 0.4f, 0.4f },
						{ { -0.5f, -5.0f }, { 0.5f, -5.0f }, { 0.5f, 5.0f }, { -0.5f, 5.0f }, { -0.5f, -5.0f } },
						{ 1.0f, 1.0f }, { 0.0f, glm::two_pi<float>() }, { 0.5f, 1.0f }),
						TCM::Texture(roseTexture), Tools::CreateTexCoordOfRectangle());
					Globals::Components().walls().back().subsequence.back().modelMatrixF = [wallId = Globals::Components().walls().size() - 1]() {
						return Globals::Components().walls()[wallId].getModelMatrix(); };
				};

				auto& wall1Body = *Globals::Components().walls().emplace_back(
					Tools::CreateBoxBody({ 5.0f, -5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f), TCM::Texture(woodTexture),
					Globals::Components().renderingSetups().size() - 1, RenderLayer::NearMidground).body;
				wall1Body.GetFixtureList()->SetRestitution(0.5f);
				setRenderingSetupAndSubsequence();

				auto& wall2Body = *Globals::Components().walls().emplace_back(
					Tools::CreateBoxBody({ 5.0f, 5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f), TCM::Texture(woodTexture),
					std::nullopt, RenderLayer::NearMidground).body;
				wall2Body.GetFixtureList()->SetRestitution(0.5f);
				setRenderingSetupAndSubsequence();

				Tools::CreateRevoluteJoint(wall1Body, wall2Body, { 5.0f, 0.0f });
			}

			const auto blendingTexture = Globals::Components().blendingTextures().size();
			Globals::Components().blendingTextures().push_back({ fractalTexture, woodTexture, spaceRockTexture, foiledEggsTexture });

			for (const float pos : {-30.0f, 30.0f})
			{
				Globals::Components().walls().emplace_back(Tools::CreateCircleBody({ 0.0f, pos }, 5.0f, b2_dynamicBody, 0.01f), TCM::Texture(),
					Globals::Components().renderingSetups().size(), RenderLayer::Midground, Globals::Shaders().texturedColorThreshold().getProgramId());

				Globals::Components().renderingSetups().emplace_back([=, this, wallId = Globals::Components().walls().size() - 1](auto) {
					Tools::MVPInitialization(Globals::Shaders().texturedColorThreshold(), Globals::Components().walls()[wallId].getModelMatrix());

					if (pos < 0.0f)
					{
						Tools::StaticTexturedRenderInitialization(Globals::Shaders().texturedColorThreshold(), orbTexture, true);
						Globals::Shaders().texturedColorThreshold().texturesCustomTransform(Tools::TextureTransform({ 0.0f, 0.0f }, 0.0f, { 5.0f, 5.0f }));
					}
					else
					{
						Tools::BlendingTexturedRenderInitialization(Globals::Shaders().texturedColorThreshold(), blendingTexture, true);
					}

					const float simulationDuration = Globals::Components().physics().simulationDuration;
					Globals::Shaders().texturedColorThreshold().invisibleColor({ 0.0f, 0.0f, 0.0f });
					Globals::Shaders().texturedColorThreshold().invisibleColorThreshold((-glm::cos(simulationDuration * 0.5f) + 1.0f) * 0.5f);
					return [=]() mutable { Globals::Shaders().texturedColorThreshold().texturesCustomTransform(glm::mat4(1.0f)); };
					});

				auto& wall = Globals::Components().walls().emplace_back(Tools::CreateCircleBody({ pos, 0.0f }, 10.0f, b2_dynamicBody, 0.01f), TCM::Texture(0));
				wall.renderLayer = RenderLayer::NearMidground;
				wall.render = false;

				Globals::Components().renderingSetups().emplace_back([
						wallId = Globals::Components().walls().size() - 1,
						texturedProgram = Shaders::Programs::TexturedAccessor()
					](Shaders::ProgramId program) mutable {
						if (!texturedProgram.isValid()) texturedProgram = program;
						texturedProgram.color(glm::vec4(
							glm::sin(Globals::Components().physics().simulationDuration* glm::two_pi<float>() * 0.2f) + 1.0f) / 2.0f);
						texturedProgram.model(Globals::Components().walls()[wallId].getModelMatrix());
						return [=]() mutable { texturedProgram.color(Globals::Components().graphicsSettings().defaultColor); };
					});

				wall.subsequence.emplace_back(Tools::CreateVerticesOfFunctionalRectangles({ 1.0f, 1.0f },
					[](float input) { return glm::vec2(glm::cos(input * 100.0f) * input * 10.0f, glm::sin(input * 100.0f) * input * 10.0f); },
					[](float input) { return glm::vec2(input + 0.3f, input + 0.3f); },
					[](float input) { return input * 600.0f; },
					[value = 0.0f]() mutable->std::optional<float> {
						if (value > 1.0f) return std::nullopt;
						float result = value;
						value += 0.002f;
						return result;
					}), TCM::Texture(roseTexture), Tools::CreateTexCoordOfRectangle(), Globals::Components().renderingSetups().size() - 1);
			}

			Globals::Components().walls().back().resolutionMode = ResolutionMode::PixelArtBlend0;
			lowResBodies.insert(Globals::Components().walls().back().body.get());
		}

		void createStationaryWalls()
		{
			const float levelWidthHSize = 80.0f;
			const float levelHeightHSize = 50.0f;
			const float bordersHGauge = 100.0f;

			auto renderingSetup = Globals::Components().renderingSetups().size();
			Globals::Components().renderingSetups().emplace_back([
				alphaFromBlendingTextureUniform = Uniforms::Uniform1b(),
					colorAccumulationUniform = Uniforms::Uniform1b(),
					texturesCustomTransform = Uniforms::UniformMat4fv<5>(),
					sceneCoordTextures = Uniforms::Uniform1b(),
					this
			](Shaders::ProgramId program) mutable {
				if (!alphaFromBlendingTextureUniform.isValid())
					alphaFromBlendingTextureUniform = Uniforms::Uniform1b(program, "alphaFromBlendingTexture");
				if (!colorAccumulationUniform.isValid())
					colorAccumulationUniform = Uniforms::Uniform1b(program, "colorAccumulation");
				if (!texturesCustomTransform.isValid())
					texturesCustomTransform = Uniforms::UniformMat4fv<5>(program, "texturesCustomTransform");
				if (!sceneCoordTextures.isValid())
					sceneCoordTextures = Uniforms::Uniform1b(program, "sceneCoordTextures");

				alphaFromBlendingTextureUniform(true);
				colorAccumulationUniform(true);
				sceneCoordTextures(true);

				texturesCustomTransform(0, glm::rotate(glm::mat4(1.0f), -textureAngle, { 0.0f, 0.0f, 1.0f }));

				for (int i = 1; i < 4; ++i)
				{
					texturesCustomTransform(i, glm::rotate(glm::mat4(1.0f), textureAngle * i / 4, { 0.0f, 0.0f, 1.0f }));
				}

				return [=]() mutable
				{
					alphaFromBlendingTextureUniform(false);
					colorAccumulationUniform(false);
					sceneCoordTextures(false);

					texturesCustomTransform(glm::mat4(1.0f));
				};
			});

			const auto blendingTexture = Globals::Components().blendingTextures().size();
			Globals::Components().blendingTextures().push_back({ fractalTexture, woodTexture, spaceRockTexture, foiledEggsTexture });

			Globals::Components().walls().emplace_back(Tools::CreateBoxBody({ -levelWidthHSize - bordersHGauge, 0.0f },
				{ bordersHGauge, levelHeightHSize + bordersHGauge * 2 }), TCM::BlendingTexture(blendingTexture), renderingSetup, RenderLayer::NearMidground).preserveTextureRatio = true;
			Globals::Components().walls().emplace_back(Tools::CreateBoxBody({ levelWidthHSize + bordersHGauge, 0.0f },
				{ bordersHGauge, levelHeightHSize + bordersHGauge * 2 }), TCM::BlendingTexture(blendingTexture), renderingSetup, RenderLayer::NearMidground).preserveTextureRatio = true;
			Globals::Components().walls().emplace_back(Tools::CreateBoxBody({ 0.0f, -levelHeightHSize - bordersHGauge },
				{ levelHeightHSize + bordersHGauge * 2, bordersHGauge }), TCM::BlendingTexture(blendingTexture), renderingSetup, RenderLayer::NearMidground).preserveTextureRatio = true;
			Globals::Components().walls().emplace_back(Tools::CreateBoxBody({ 0.0f, levelHeightHSize + bordersHGauge },
				{ levelHeightHSize + bordersHGauge * 2, bordersHGauge }), TCM::BlendingTexture(blendingTexture), renderingSetup, RenderLayer::NearMidground).preserveTextureRatio = true;

			renderingSetup = Globals::Components().renderingSetups().size();
			Globals::Components().renderingSetups().emplace_back([
				playerUnhidingRadiusUniform = Uniforms::Uniform1f(),
				this
			](Shaders::ProgramId program) mutable {
					if (!playerUnhidingRadiusUniform.isValid())
						playerUnhidingRadiusUniform = Uniforms::Uniform1f(program, "playerUnhidingRadius");

					playerUnhidingRadiusUniform(20.0f);

					return [=]() mutable
					{
						playerUnhidingRadiusUniform(0.0f);
					};
				});

			Globals::Components().decorations().emplace_back(Tools::CreateVerticesOfLineOfRectangles({ 1.5f, 1.5f },
				{ { -levelWidthHSize, -levelHeightHSize }, { levelWidthHSize, -levelHeightHSize }, { levelWidthHSize, levelHeightHSize },
				{ -levelWidthHSize, levelHeightHSize }, { -levelWidthHSize, -levelHeightHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), TCM::Texture(weedTexture), Tools::CreateTexCoordOfRectangle(), renderingSetup);
			Globals::Components().decorations().back().renderLayer = RenderLayer::FarForeground;
			//Globals::Components().decorations().back().resolutionMode = ResolutionMode::PixelArtBlend0;
		}

		void createGrapples() const
		{
			EmplaceDynamicComponent(Globals::Components().grapples(), { Tools::CreateCircleBody({ 0.0f, 10.0f }, 1.0f), TCM::Texture(orbTexture) }).influenceRadius = 15.0f;

			Globals::Components().renderingSetups().emplace_back([
				colorUniform = Uniforms::Uniform4f()
			](Shaders::ProgramId program) mutable {
					if (!colorUniform.isValid()) colorUniform = Uniforms::Uniform4f(program, "color");
					colorUniform(glm::vec4((glm::sin(Globals::Components().physics().simulationDuration / 3.0f * glm::two_pi<float>()) + 1.0f) / 2.0f));
					return [=]() mutable { colorUniform(Globals::Components().graphicsSettings().defaultColor); };
				});

			EmplaceDynamicComponent(Globals::Components().grapples(), { Tools::CreateCircleBody({ 0.0f, -10.0f }, 1.0f), TCM::Texture(orbTexture),
				Globals::Components().renderingSetups().size() - 1 }).influenceRadius = 15.0f;

			Globals::Components().renderingSetups().emplace_back([
				texturesCustomTransformUniform = Uniforms::UniformMat4f()
			](Shaders::ProgramId program) mutable {
					if (!texturesCustomTransformUniform.isValid()) texturesCustomTransformUniform = Uniforms::UniformMat4f(program, "texturesCustomTransform");
					texturesCustomTransformUniform(Tools::TextureTransform({ 0.0f, 0.0f }, 0.0f, { 2.0f, 2.0f }));
					return [=]() mutable { texturesCustomTransformUniform(glm::mat4(1.0f)); };
				});

			EmplaceDynamicComponent(Globals::Components().grapples(), { Tools::CreateCircleBody({ -10.0f, -30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f),
				TCM::Texture(orbTexture), Globals::Components().renderingSetups().size() - 1 }).influenceRadius = 30.0f;

			auto& grapple = EmplaceDynamicComponent(Globals::Components().grapples(), { Tools::CreateCircleBody({ -10.0f, 30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f), TCM::Texture(0) });
			grapple.influenceRadius = 30.0f;
			grapple.render = false;
			grapple.subsequence.emplace_back(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 2.2f, 2.2f }),
				TCM::Texture(roseTexture), Tools::CreateTexCoordOfRectangle());
			grapple.subsequence.back().modelMatrixF = [&grapple]() {
				return grapple.getModelMatrix();
			};
		}

		void setCamera() const
		{
			const float transitionFactor = 10.0f;
			const auto& planes = Globals::Components().planes();
			const auto& screenInfo = Globals::Components().screenInfo();

			auto velocityCorrection = [](const auto& plane) {
				return plane.getVelocity() * 0.2f;
			};

			Globals::Components().camera().targetProjectionHSizeF = [&, transitionFactor]() {
				Globals::Components().camera().projectionTransitionFactor = Globals::Components().physics().frameDuration * transitionFactor;

				const float maxDistance = [&]() {
					const float scalingFactor = 0.6f;
					float maxDistance = 0.0f;
					for (unsigned i = 0; i < playersHandlers.size() - 1; ++i)
						for (unsigned j = i + 1; j < playersHandlers.size(); ++j)
						{
							const auto& plane1 = planes.at(playersHandlers[i].playerId);
							const auto& plane2 = planes.at(playersHandlers[j].playerId);
							const auto plane1Center = plane1.getCenter() + velocityCorrection(plane1);
							const auto plane2Center = plane2.getCenter() + velocityCorrection(plane2);
							/*maxDistance = std::max(maxDistance, glm::max(glm::abs(plane1Center.x - plane2Center.x) * screenInfo.windowSize.y / screenInfo.windowSize.x * scalingFactor,
								glm::abs(plane1Center.y - plane2Center.y) * scalingFactor));*/
							maxDistance = std::max(maxDistance, glm::distance(glm::vec2(plane1Center.x * screenInfo.windowSize.y / screenInfo.windowSize.x, plane1Center.y),
								glm::vec2(plane2Center.x * screenInfo.windowSize.y / screenInfo.windowSize.x, plane2Center.y)) * scalingFactor);
						}
					return maxDistance;
				}();

				return std::max(projectionHSizeBase, maxDistance);
			};

			Globals::Components().camera().targetPositionF = [&, transitionFactor]() {
				Globals::Components().camera().positionTransitionFactor = Globals::Components().physics().frameDuration * transitionFactor;

				glm::vec2 min(std::numeric_limits<float>::max());
				glm::vec2 max(std::numeric_limits<float>::lowest());
				glm::vec2 sumOfVelocityCorrections(0.0f);

				for (const auto& playerHandler : playersHandlers)
				{
					const auto& plane = planes.at(playerHandler.playerId);
					min = { std::min(min.x, plane.getCenter().x), std::min(min.y, plane.getCenter().y) };
					max = { std::max(max.x, plane.getCenter().x), std::max(max.y, plane.getCenter().y) };
					sumOfVelocityCorrections += velocityCorrection(plane);
				}

				return (min + max) / 2.0f + sumOfVelocityCorrections / (float)playersHandlers.size();
			};
		}

		void setCollisionFilters()
		{
			EmplaceDynamicComponent(Globals::Components().collisionFilters(), { Globals::CollisionBits::missile, Globals::CollisionBits::missile | Globals::CollisionBits::plane,
				[this](const auto& missileFixture, const auto& targetFixture) {
					const auto missileId = std::get<TCM::Missile>(Tools::AccessUserData(*missileFixture.GetBody()).bodyComponentVariant).id;
					const auto& targetBodyComponentVariant = Tools::AccessUserData(*targetFixture.GetBody()).bodyComponentVariant;
					const auto missilePlaneId = missilesToHandlers.at(missileId).planeId;

					if (!missilePlaneId)
						return true;

					if (const TCM::Missile* targetMissile = std::get_if<TCM::Missile>(&targetBodyComponentVariant))
					{
						const auto targetMissilePlaneId = missilesToHandlers.at(targetMissile->id).planeId;
						return !targetMissilePlaneId || *missilePlaneId != *targetMissilePlaneId;
					}

					return *missilePlaneId != std::get<TCM::Plane>(targetBodyComponentVariant).id;
				}
			});
		}

		void setCollisionCallbacks()
		{
			EmplaceDynamicComponent(Globals::Components().beginCollisionHandlers(), { Globals::CollisionBits::missile, Globals::CollisionBits::all,
				[this](const auto& missileFixture, const auto& targetFixture) {
					auto& deferredActions = Globals::Components().deferredActions();
					const auto& missileBody = *missileFixture.GetBody();

					deferredActions.emplace_back([&](auto) {
						missilesToHandlers.erase(std::get<TCM::Missile>(Tools::AccessUserData(missileBody).bodyComponentVariant).id);
						return false;
						});

					Tools::CreateExplosion(Tools::ExplosionParams().center(ToVec2<glm::vec2>(missileBody.GetWorldCenter())).explosionTexture(explosionTexture).resolutionMode(
						lowResBodies.count(targetFixture.GetBody()) ? ResolutionMode::LowPixelArtBlend1 : ResolutionMode::LowestLinearBlend1));

					const auto& targetBodyComponentVariant = Tools::AccessUserData(*targetFixture.GetBody()).bodyComponentVariant;
					if (const TCM::Missile* targetMissile = std::get_if<TCM::Missile>(&targetBodyComponentVariant))
					{
						deferredActions.emplace_back([=](auto) {
							missilesToHandlers.erase(targetMissile->id);
							return false;
							});

						Tools::CreateExplosion(Tools::ExplosionParams().center(ToVec2<glm::vec2>(targetFixture.GetBody()->GetWorldCenter())).explosionTexture(explosionTexture));
					}

					explosionFrame = true;
				}
			});
		}

		void setFramesRoutines()
		{
			EmplaceDynamicComponent(Globals::Components().frameSetups(), { [&]()
				{
					explosionFrame = false;
				} });
		}

		void createSpawners()
		{
			const auto alpha = std::make_shared<float>(0.0f);
			const unsigned renderingSetupId = Globals::Components().renderingSetups().size();
			Globals::Components().renderingSetups().emplace_back([=,
				colorUniform = Uniforms::Uniform4f()
			](Shaders::ProgramId program) mutable {
					if (!colorUniform.isValid()) colorUniform = Uniforms::Uniform4f(program, "color");
					colorUniform(glm::vec4(*alpha));
					return [=]() mutable { colorUniform(Globals::Components().graphicsSettings().defaultColor); };
				});

			auto spawner = [this, alpha, renderingSetupId, first = true](float duration, auto& spawner) mutable -> bool //Type deduction doesn't get it is always bool.
			{
				const float existenceDuration = 2.0f;
				const float fadeDuration = 0.2f;

				*alpha = duration < fadeDuration
					? duration / fadeDuration
					: duration > (existenceDuration - fadeDuration)
					? (existenceDuration - duration) / fadeDuration
					: 1.0f;

				if (first)
				{
					first = false;
					auto& wall = EmplaceDynamicComponent(Globals::Components().dynamicWalls(), { Tools::CreateBoxBody({ -50.0f, 30.0f }, { 5.0f, 5.0f }),
						TCM::Texture(woodTexture, { 0.0f, 0.0f }, 0.0f, { 5.0f, 5.0f }), renderingSetupId });
					dynamicWallId = wall.getComponentId();

					auto& grapple = EmplaceDynamicComponent(Globals::Components().grapples(), { Tools::CreateCircleBody({ 50.0f, 30.0f }, existenceDuration),
						TCM::Texture(orbTexture, { 0.0f, 0.0f }, 0.0f, { 2.0f, 2.0f }), renderingSetupId });
					grapple.influenceRadius = 20.0f;
					dynamicGrappleId = grapple.getComponentId();
				}
				else if (duration >= existenceDuration)
				{
					first = true;
					Globals::Components().dynamicWalls()[dynamicWallId].state = ComponentState::Outdated;
					Globals::Components().grapples()[dynamicGrappleId].state = ComponentState::Outdated;
					Globals::Components().deferredActions().emplace_back([spawner](float duration) mutable { return spawner(duration, spawner); }, existenceDuration);

					return false;
				}

				return true;
			};
			Globals::Components().deferredActions().emplace_back([spawner](float duration) mutable { return spawner(duration, spawner); });
		}

		void playersControls()
		{
			const float mouseSensitivity = 0.01f;
			const float gamepadSensitivity = 50.0f;

			const auto& physics = Globals::Components().physics();
			const auto& mouse = Globals::Components().mouse();
			const auto& gamepads = Globals::Components().gamepads();

			for (unsigned i = 0; i < playersHandlers.size(); ++i)
			{
				const auto gamepadId = playersHandlers[i].gamepadId;
				auto& playerControls = Globals::Components().planes()[playersHandlers[i].playerId].controls;
				bool fire = false;

				playerControls = Components::Plane::Controls();

				if (i == 0)
				{
					playerControls.turningDelta = mouse.getWorldSpaceDelta() * mouseSensitivity;
					playerControls.autoRotation = (bool)mouse.pressing.rmb;
					playerControls.throttling = (float)mouse.pressing.rmb;
					playerControls.magneticHook = mouse.pressing.xmb1;
					fire = mouse.pressing.lmb;
				}

				if (gamepadId)
				{
					const auto& gamepad = gamepads[*gamepadId];

					playerControls.turningDelta += Tools::ApplyDeadzone(gamepad.lStick) * physics.frameDuration * gamepadSensitivity;
					playerControls.autoRotation |= (bool)gamepad.rTrigger;
					playerControls.throttling = std::max(gamepad.rTrigger, playerControls.throttling);
					playerControls.magneticHook |= gamepad.pressing.lShoulder || gamepad.pressing.a || gamepad.lTrigger >= 0.5f;
					fire |= gamepad.pressing.x;
				}

				launchingMissile(i, fire);
			}
		}

		void step()
		{
			const auto& mouse = Globals::Components().mouse();
			const auto& gamepads = Globals::Components().gamepads();

			updateNumOfPlayers(false);
			playersControls();
			
			if (mouse.pressing.mmb || gamepads[0].pressing.rShoulder)
				Globals::Components().physics().gameSpeed = std::clamp(Globals::Components().physics().gameSpeed +
					(mouse.pressed.wheel + gamepads[0].pressed.dUp * 1 + gamepads[0].pressed.dDown * -1) * 0.1f, 0.0f, 2.0f);
			else
				projectionHSizeBase = std::clamp(projectionHSizeBase + (mouse.pressed.wheel +
					gamepads[0].pressed.dUp * 1 + gamepads[0].pressed.dDown * -1) * -5.0f, 5.0f, 100.0f);

			//textureAngle += Globals::Components().physics().frameDuration * 0.2f;
		}

	private:
		unsigned rocketPlaneTexture = 0;
		unsigned spaceRockTexture = 0;
		unsigned woodTexture = 0;
		unsigned orbTexture = 0;
		unsigned weedTexture = 0;
		unsigned roseTexture = 0;
		unsigned fogTexture = 0;
		unsigned flameAnimationTexture = 0;
		unsigned missile1Texture = 0;
		unsigned missile2Texture = 0;
		unsigned explosionTexture = 0;
		unsigned foiledEggsTexture = 0;
		unsigned fractalTexture = 0;
		unsigned mosaicTexture = 0;
		unsigned ppTexture = 0;
		unsigned skullTexture = 0;
		unsigned avatarTexture = 0;

		std::array<unsigned, 4> flameAnimatedTextureForPlayers{ 0 };
		unsigned flameAnimatedTexture = 0;
		unsigned invertedFlameAnimatedTexture = 0;

		struct PlayerHandler
		{
			unsigned playerId = 0;
			std::optional<unsigned> gamepadId;
			float durationToLaunchMissile = 0.0f;
		};

		std::vector<PlayerHandler> playersHandlers;

		float projectionHSizeBase = 20.0f;

		bool explosionFrame = false;

		float fogAlphaFactor = 1.0f;
		float textureAngle = 0.0f;

		unsigned dynamicWallId = 0;
		unsigned dynamicGrappleId = 0;

		std::unordered_map<ComponentId, Tools::MissileHandler> missilesToHandlers;
		std::unordered_set<const b2Body*> lowResBodies;
	};

	Playground::Playground():
		impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->setAnimations();
		impl->createBackground();
		impl->updateNumOfPlayers(true);
		impl->createMovableWalls();
		impl->createStationaryWalls();
		impl->createGrapples();
		impl->createForeground();
		impl->createAdditionalDecorations();
		impl->setCamera();
		impl->setCollisionFilters();
		impl->setCollisionCallbacks();
		impl->setFramesRoutines();
		impl->createSpawners();
	}

	Playground::~Playground() = default;

	void Playground::step()
	{
		impl->step();
	}
}
