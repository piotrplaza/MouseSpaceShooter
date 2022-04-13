#include "playground.hpp"

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
#include <components/missile.hpp>
#include <components/collisionHandler.hpp>
#include <components/shockwave.hpp>
#include <components/functor.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/blendingTexture.hpp>
#include <components/animatedTexture.hpp>

#include <ogl/uniformControllers.hpp>
#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/julia.hpp>
#include <ogl/shaders/texturedColorThreshold.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/shaders/particles.hpp>
#include <ogl/renderingHelpers.hpp>

#include <globals/shaders.hpp>
#include <globals/components.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>

#include <commonIds/collisionBits.hpp>

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace Levels
{
	class Playground::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			Globals::Components().graphicsSettings().defaultColor = { 0.7f, 0.7f, 0.7f, 1.0f };
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

			flame1AnimationTexture = textures.size();
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
			flame1AnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().push_back(Components::AnimatedTexture(
				flame1AnimationTexture, { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
				AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal));
			Globals::Components().animatedTextures().back().start(true);

			flame2AnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().push_back(Globals::Components().animatedTextures().back());
			Globals::Components().animatedTextures().back().setAdditionalTransformation({ 0.0f, 0.0f }, glm::pi<float>());
		}

		void createBackground()
		{
			Tools::CreateJuliaBackground(Globals::Shaders().julia(), []() { return Globals::Components().players()[1].getCenter() * 0.0001f; });
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
			Globals::Components().blendingTextures().push_back({ { flame2AnimatedTexture, ppTexture, skullTexture, avatarTexture }, true });

			for (int i = 0; i < 2; ++i)
			{
				glm::vec2 portraitCenter(i == 0 ? -40.0f : 40.0f, -40.0f);
				Globals::Components().renderingSetups().emplace_back([=,
					addBlendingColor = Uniforms::UniformController4f()
				](Shaders::ProgramId program) mutable {
					if (!addBlendingColor.isValid())
						addBlendingColor = Uniforms::UniformController4f(program, "addBlendingColor");

					const float skullOpacity = fogAlphaFactor - 1.0f;
					const float avatarOpacity = glm::min(0.0f, glm::distance(Globals::Components().players()[1].getCenter(), portraitCenter) / 3.0f - 5.0f);

					i == 0
						? addBlendingColor({ 1.0f, skullOpacity, avatarOpacity, 0.0f })
						: addBlendingColor({ 0.0f, 0.0f, 1.0f, 0.0f });

					return [=]() mutable {
						addBlendingColor({ 0.0f, 0.0f, 0.0f, 0.0f });
					};
				});

				Globals::Components().midgroundDecorations().emplace_back(Tools::CreatePositionsOfRectangle(portraitCenter, { 10.0f, 10.0f }),
					TCM::BlendingTexture(blendingTexture), Tools::CreateTexCoordOfRectangle(), Globals::Components().renderingSetups().size() - 1).preserveTextureRatio = true;
			}
		}

		void createPlayers()
		{
			player1Handler = Tools::CreatePlayerPlane(rocketPlaneTexture, flame1AnimatedTexture, { -10.0f, 0.0f });
		}

		void launchMissile()
		{
			auto missileHandler = Tools::CreateMissile(Globals::Components().players()[1].getCenter(),
				Globals::Components().players()[1].getAngle(), 5.0f, Globals::Components().players()[1].getVelocity(),
				missile2Texture, flame1AnimatedTexture);
			missilesToHandlers.emplace(missileHandler.missileId, std::move(missileHandler));
		}

		void createDynamicWalls()
		{
			Globals::Components().renderingSetups().emplace_back([
				texturesCustomTransformUniform = Uniforms::UniformControllerMat4f()
			](Shaders::ProgramId program) mutable {
					if (!texturesCustomTransformUniform.isValid()) texturesCustomTransformUniform = Uniforms::UniformControllerMat4f(program, "texturesCustomTransform");
					const float simulationDuration = Globals::Components().physics().simulationDuration;
					texturesCustomTransformUniform(Tools::TextureTransform(glm::vec2(glm::cos(simulationDuration), glm::sin(simulationDuration)) * 0.1f ));
					return [=]() mutable { texturesCustomTransformUniform(glm::mat4(1.0f)); };
				});

			auto& wall1 = *Globals::Components().dynamicWalls().emplace_back(
				Tools::CreateBoxBody({ 5.0f, -5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f), TCM::Texture(woodTexture), Globals::Components().renderingSetups().size() - 1).body;
			auto& wall2 = *Globals::Components().dynamicWalls().emplace_back(
				Tools::CreateBoxBody({ 5.0f, 5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f), TCM::Texture(woodTexture)).body;
			wall1.GetFixtureList()->SetRestitution(0.5f);
			wall2.GetFixtureList()->SetRestitution(0.5f);
			Tools::PinBodies(wall1, wall2, { 5.0f, 0.0f });

			for (int i = 1; i <= 2; ++i)
			{
				Globals::Components().renderingSetups().emplace_back([
					texturedProgram = Shaders::Programs::TexturedAccessor(),
						wallId = Globals::Components().dynamicWalls().size() - i
				](Shaders::ProgramId program) mutable {
						if (!texturedProgram.isValid()) texturedProgram = program;
						texturedProgram.model(Globals::Components().dynamicWalls()[wallId].getModelMatrix());
						return nullptr;
					});

				Globals::Components().midgroundDecorations().emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 0.4f, 0.4f },
					{ { -0.5f, -5.0f }, { 0.5f, -5.0f }, { 0.5f, 5.0f }, { -0.5f, 5.0f}, { -0.5f, -5.0f } },
					{ 1.0f, 1.0f }, { 0.0f, glm::two_pi<float>() }, { 0.5f, 1.0f }),
					TCM::Texture(roseTexture), Tools::CreateTexCoordOfRectangle(), Globals::Components().renderingSetups().size() - 1);
			}

			const auto blendingTexture = Globals::Components().blendingTextures().size();
			Globals::Components().blendingTextures().push_back({ fractalTexture, woodTexture, spaceRockTexture, foiledEggsTexture });

			for (const float pos : {-30.0f, 30.0f})
			{
				Globals::Components().renderingSetups().emplace_back([=, this](auto) {
						Tools::MVPInitialization(Globals::Shaders().texturedColorThreshold());
						if (pos < 0.0f)
						{
							Tools::StaticTexturedRenderInitialization(Globals::Shaders().texturedColorThreshold(), orbTexture, true);
							Globals::Shaders().texturedColorThreshold().texturesCustomTransform(Tools::TextureTransform({ 0.0f, 0.0f }, 0.0f, { 5.0f, 5.0f }));
						}
						else
							Tools::BlendingTexturedRenderInitialization(Globals::Shaders().texturedColorThreshold(), blendingTexture, true);
						const float simulationDuration = Globals::Components().physics().simulationDuration;
						Globals::Shaders().texturedColorThreshold().invisibleColor({ 0.0f, 0.0f, 0.0f });
						Globals::Shaders().texturedColorThreshold().invisibleColorThreshold((-glm::cos(simulationDuration * 0.5f) + 1.0f) * 0.5f);
						return [=]() mutable { Globals::Shaders().texturedColorThreshold().texturesCustomTransform(glm::mat4(1.0f)); };
					});

				Globals::Components().dynamicWalls().emplace_back(Tools::CreateCircleBody({ 0.0f, pos }, 5.0f, b2_dynamicBody, 0.01f), TCM::Texture(),
					Globals::Components().renderingSetups().size() - 1, Globals::Shaders().texturedColorThreshold().getProgramId());

				Globals::Components().renderingSetups().emplace_back([
						basicProgram = Shaders::Programs::BasicAccessor()
					](Shaders::ProgramId program) mutable {
						if (!basicProgram.isValid()) basicProgram = program;
						basicProgram.color(glm::vec4(0.0f));
						return nullptr;
					});

				Globals::Components().dynamicWalls().emplace_back(Tools::CreateCircleBody({ pos, 0.0f }, 10.0f, b2_dynamicBody, 0.01f));
				Globals::Components().dynamicWalls().back().renderingSetup = Globals::Components().renderingSetups().size() - 1;

				Globals::Components().renderingSetups().emplace_back([
						wallId = Globals::Components().dynamicWalls().size() - 1,
						texturedProgram = Shaders::Programs::TexturedAccessor()
					](Shaders::ProgramId program) mutable {
						if (!texturedProgram.isValid()) texturedProgram = program;
						texturedProgram.color(glm::vec4(
							glm::sin(Globals::Components().physics().simulationDuration* glm::two_pi<float>() * 0.2f) + 1.0f) / 2.0f);
						texturedProgram.model(Globals::Components().dynamicWalls()[wallId].getModelMatrix());
						return nullptr;
					});

				Globals::Components().midgroundDecorations().emplace_back(Tools::CreatePositionsOfFunctionalRectangles({ 1.0f, 1.0f },
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

			Globals::Components().midgroundDecorations().back().resolutionMode = ResolutionMode::PixelArtBlend0;
			lowResBodies.insert(Globals::Components().dynamicWalls().back().body.get());
		}

		void createStaticWalls()
		{
			const float levelHSize = 50.0f;
			const float bordersHGauge = 100.0f;

			const auto renderingSetup = Globals::Components().renderingSetups().size();
			Globals::Components().renderingSetups().emplace_back([
				alphaFromBlendingTextureUniform = Uniforms::UniformController1b(),
					colorAccumulationUniform = Uniforms::UniformController1b(),
					texturesCustomTransform = Uniforms::UniformControllerMat4fv<5>(),
					this
			](Shaders::ProgramId program) mutable {
				if (!alphaFromBlendingTextureUniform.isValid())
					alphaFromBlendingTextureUniform = Uniforms::UniformController1b(program, "alphaFromBlendingTexture");
				if (!colorAccumulationUniform.isValid())
					colorAccumulationUniform = Uniforms::UniformController1b(program, "colorAccumulation");
				if (!texturesCustomTransform.isValid())
					texturesCustomTransform = Uniforms::UniformControllerMat4fv<5>(program, "texturesCustomTransform");

				alphaFromBlendingTextureUniform(true);
				colorAccumulationUniform(true);

				texturesCustomTransform(0, glm::rotate(glm::mat4(1.0f), -textureAngle, { 0.0f, 0.0f, 1.0f }));

				for (int i = 1; i < 4; ++i)
				{
					texturesCustomTransform(i, glm::rotate(glm::mat4(1.0f), textureAngle * i / 4, { 0.0f, 0.0f, 1.0f }));
				}

				return [=]() mutable
				{
					alphaFromBlendingTextureUniform(false);
					colorAccumulationUniform(false);
				};
			});

			const auto blendingTexture = Globals::Components().blendingTextures().size();
			Globals::Components().blendingTextures().push_back({ fractalTexture, woodTexture, spaceRockTexture, foiledEggsTexture });

			Globals::Components().staticWalls().emplace_back(Tools::CreateBoxBody({ -levelHSize - bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize + bordersHGauge * 2 }), TCM::BlendingTexture(blendingTexture), renderingSetup).preserveTextureRatio = true;
			Globals::Components().staticWalls().emplace_back(Tools::CreateBoxBody({ levelHSize + bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize + bordersHGauge * 2 }), TCM::BlendingTexture(blendingTexture), renderingSetup).preserveTextureRatio = true;
			Globals::Components().staticWalls().emplace_back(Tools::CreateBoxBody({ 0.0f, -levelHSize - bordersHGauge },
				{ levelHSize + bordersHGauge * 2, bordersHGauge }), TCM::BlendingTexture(blendingTexture), renderingSetup).preserveTextureRatio = true;
			Globals::Components().staticWalls().emplace_back(Tools::CreateBoxBody({ 0.0f, levelHSize + bordersHGauge },
				{ levelHSize + bordersHGauge * 2, bordersHGauge }), TCM::BlendingTexture(blendingTexture), renderingSetup).preserveTextureRatio = true;

			Globals::Components().nearMidgroundDecorations().emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.5f, 1.5f },
				{ { -levelHSize, -levelHSize }, { levelHSize, -levelHSize }, { levelHSize, levelHSize }, { -levelHSize, levelHSize }, { -levelHSize, -levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), TCM::Texture(weedTexture), Tools::CreateTexCoordOfRectangle());
			//Globals::Components().nearMidgroundDecorations().back().resolutionMode = ResolutionMode::PixelArtBlend0;
		}

		void createGrapples() const
		{
			Globals::Components().grapples().emplace_back(Tools::CreateCircleBody({ 0.0f, 10.0f }, 1.0f), 15.0f, TCM::Texture(orbTexture));

			Globals::Components().renderingSetups().emplace_back([
				colorUniform = Uniforms::UniformController4f()
			](Shaders::ProgramId program) mutable {
					if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
					colorUniform(glm::vec4(glm::sin(Globals::Components().physics().simulationDuration / 3.0f * glm::two_pi<float>()) + 1.0f) / 2.0f);
					return nullptr;
				});

			Globals::Components().grapples().emplace_back(Tools::CreateCircleBody({ 0.0f, -10.0f }, 1.0f), 15.0f, TCM::Texture(orbTexture),
				Globals::Components().renderingSetups().size() - 1);

			Globals::Components().renderingSetups().emplace_back([
				texturesCustomTransformUniform = Uniforms::UniformControllerMat4f()
			](Shaders::ProgramId program) mutable {
					if (!texturesCustomTransformUniform.isValid()) texturesCustomTransformUniform = Uniforms::UniformControllerMat4f(program, "texturesCustomTransform");
					texturesCustomTransformUniform(Tools::TextureTransform({ 0.0f, 0.0f }, 0.0f, { 2.0f, 2.0f }));
					return [=]() mutable { texturesCustomTransformUniform(glm::mat4(1.0f)); };
				});

			Globals::Components().grapples().emplace_back(Tools::CreateCircleBody({ -10.0f, -30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f), 30.0f,
				TCM::Texture(orbTexture), Globals::Components().renderingSetups().size() - 1);

			Globals::Components().renderingSetups().emplace_back([
				colorUniform = Uniforms::UniformController4f()
			](Shaders::ProgramId program) mutable {
					if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
					colorUniform({ 0.0f, 0.0f, 0.0f, 0.0f });
					return nullptr;
				});

			auto& grapple = Globals::Components().grapples().emplace_back(Tools::CreateCircleBody({ -10.0f, 30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f), 30.0f);
			grapple.renderingSetup = Globals::Components().renderingSetups().size() - 1;

			Globals::Components().renderingSetups().emplace_back([&,
				modelUniform = Uniforms::UniformControllerMat4f()
			](Shaders::ProgramId program) mutable {
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
				modelUniform(grapple.getModelMatrix());
				return nullptr;
			});

			Globals::Components().farMidgroundDecorations().emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 2.2f, 2.2f }),
				TCM::Texture(roseTexture), Tools::CreateTexCoordOfRectangle(), Globals::Components().renderingSetups().size() - 1);
		}

		void setCamera() const
		{
			const auto& player = Globals::Components().players()[player1Handler.playerId];

			Globals::Components().camera().targetProjectionHSizeF = [&]() {
				Globals::Components().camera().projectionTransitionFactor = Globals::Components().physics().frameDuration * 6;
				return projectionHSizeBase + glm::length(player.getVelocity()) * 0.2f;
			};
			Globals::Components().camera().targetPositionF = [&]() {
				Globals::Components().camera().positionTransitionFactor = Globals::Components().physics().frameDuration * 6;
				return player.getCenter() + glm::vec2(glm::cos(player.getAngle()), glm::sin(player.getAngle())) * 5.0f + player.getVelocity() * 0.4f;
			};
		}

		void setCollisionCallbacks()
		{
			EmplaceIdComponent(Globals::Components().beginCollisionHandlers(), { CollisionBits::missileBit, CollisionBits::all,
				[this](const auto& fixtureA, const auto& fixtureB) {
					for (const auto* fixture : { &fixtureA, &fixtureB })
					if (fixture->GetFilterData().categoryBits == CollisionBits::missileBit)
					{
						const auto& otherFixture = fixture == &fixtureA ? fixtureB : fixtureA;
						const auto& body = *fixture->GetBody();
						missilesToHandlers.erase(std::get<TCM::Missile>(Tools::AccessUserData(body).bodyComponentVariant).id);
						Tools::CreateExplosion(Globals::Shaders().particles(), ToVec2<glm::vec2>(body.GetWorldCenter()), explosionTexture, 1.0f, 64, 4,
							lowResBodies.count(otherFixture.GetBody()) ? ResolutionMode::LowPixelArtBlend1 : ResolutionMode::LowestLinearBlend1);

						explosionFrame = true;
					}
				}
			});
		}

		void setFramesRoutines()
		{
			EmplaceIdComponent(Globals::Components().frameSetups(), { [&]()
				{
					explosionFrame = false;
				} });
		}

		void step()
		{
			if (Globals::Components().mouseState().lmb)
			{
				if (durationToLaunchMissile <= 0.0f)
				{
					launchMissile();
					durationToLaunchMissile = 0.1f;
				}
				else durationToLaunchMissile -= Globals::Components().physics().frameDuration;
			}
			else durationToLaunchMissile = 0.0f;

			projectionHSizeBase = std::clamp(projectionHSizeBase + (prevWheel - Globals::Components().mouseState().wheel) * 5.0f, 5.0f, 100.0f);
			prevWheel = Globals::Components().mouseState().wheel;

			//textureAngle += Globals::Components().physics().frameDuration * 0.05f;
		}

	private:
		unsigned rocketPlaneTexture = 0;
		unsigned spaceRockTexture = 0;
		unsigned woodTexture = 0;
		unsigned orbTexture = 0;
		unsigned weedTexture = 0;
		unsigned roseTexture = 0;
		unsigned fogTexture = 0;
		unsigned flame1AnimationTexture = 0;
		unsigned missile1Texture = 0;
		unsigned missile2Texture = 0;
		unsigned explosionTexture = 0;
		unsigned foiledEggsTexture = 0;
		unsigned fractalTexture = 0;
		unsigned mosaicTexture = 0;
		unsigned ppTexture = 0;
		unsigned skullTexture = 0;
		unsigned avatarTexture = 0;

		unsigned flame1AnimatedTexture = 0;
		unsigned flame2AnimatedTexture = 0;

		Tools::PlayerPlaneHandler player1Handler;

		float durationToLaunchMissile = 0.0f;

		int prevWheel = 0;
		float projectionHSizeBase = 20.0f;

		bool explosionFrame = false;

		float fogAlphaFactor = 1.0f;
		float textureAngle = 0.0f;

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
		impl->createPlayers();
		impl->createDynamicWalls();
		impl->createStaticWalls();
		impl->createGrapples();
		impl->createForeground();
		impl->createAdditionalDecorations();
		impl->setCamera();
		impl->setCollisionCallbacks();
		impl->setFramesRoutines();
	}

	Playground::~Playground() = default;

	void Playground::step()
	{
		impl->step();
	}
}
