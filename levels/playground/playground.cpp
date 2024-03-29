#include "playground.hpp"

#include <components/screenInfo.hpp>
#include <components/physics.hpp>
#include <components/renderingSetup.hpp>
#include <components/plane.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/camera2D.hpp>
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
#include <components/soundBuffer.hpp>
#include <components/sound.hpp>
#include <components/music.hpp>
#include <components/audioListener.hpp>

#include <globals/shaders.hpp>
#include <globals/components.hpp>
#include <globals/systems.hpp>
#include <globals/collisionBits.hpp>

#include <ogl/uniforms.hpp>
#include <ogl/shaders/texturedColorThreshold.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/renderingHelpers.hpp>

#include <tools/shapes2D.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>
#include <tools/playersHandler.hpp>
#include <tools/missilesHandler.hpp>
#include <tools/splines.hpp>

#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <optional>

namespace Levels
{
	class Playground::Impl
	{
	public:
		void globalSettings() const
		{
			Globals::Components().graphicsSettings().defaultColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			Globals::Components().mainFramebufferRenderer().renderer = Tools::Demo3DRotatedFullscreenRenderer(Globals::Shaders().textured());
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().staticTextures();

			planeTextures[0] = textures.size();
			textures.emplace("textures/plane 1.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.6f, 1.8f);
			textures.last().minFilter = GL_LINEAR;

			planeTextures[1] = textures.size();
			textures.emplace("textures/alien ship 1.png");
			textures.last().translate = glm::vec2(-0.2f, 0.0f);
			textures.last().scale = glm::vec2(1.9f);
			textures.last().minFilter = GL_LINEAR;

			planeTextures[2] = textures.size();
			textures.emplace("textures/plane 2.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.8f, 1.8f);
			textures.last().minFilter = GL_LINEAR;

			planeTextures[3] = textures.size();
			textures.emplace("textures/alien ship 2.png");
			textures.last().translate = glm::vec2(0.0f, 0.0f);
			textures.last().scale = glm::vec2(1.45f, 1.4f);
			textures.last().minFilter = GL_LINEAR;

			spaceRockTexture = textures.size();
			textures.emplace("textures/space rock.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(5.0f);

			woodTexture = textures.size();
			textures.emplace("textures/wood.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(5.0f);

			orbTexture = textures.size();
			textures.emplace("textures/orb.png");
			textures.last().scale = glm::vec2(2.0f);

			weedTexture = textures.size();
			textures.emplace("textures/weed.png");
			textures.last().minFilter = GL_LINEAR_MIPMAP_LINEAR;

			roseTexture = textures.size();
			textures.emplace("textures/rose.png");
			textures.last().minFilter = GL_LINEAR_MIPMAP_LINEAR;

			fogTexture = textures.size();
			textures.emplace("textures/fog.png");

			flameAnimationTexture = textures.size();
			textures.emplace("textures/flame animation 1.jpg");
			textures.last().minFilter = GL_LINEAR;

			missile1Texture = textures.size();
			textures.emplace("textures/missile 1.png");
			textures.last().minFilter = GL_LINEAR;
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(0.3f, 0.4f);

			missile2Texture = textures.size();
			textures.emplace("textures/missile 2.png");
			textures.last().minFilter = GL_LINEAR;
			textures.last().scale = glm::vec2(0.4f, 0.45f);

			explosionTexture = textures.size();
			textures.emplace("textures/explosion.png");

			foiledEggsTexture = textures.size();
			textures.emplace("textures/foiled eggs.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(100.0f, 100.0f);

			fractalTexture = textures.size();
			textures.emplace("textures/fractal.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(50.0f, 50.0f);

			mosaicTexture = textures.size();
			textures.emplace("textures/mosaic.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(1.0f, 1.0f);

			ppTexture = textures.size();
			textures.emplace("textures/pp.png");

			skullTexture = textures.size();
			textures.emplace("textures/skull rot.png");
			textures.last().translate = glm::vec2(0.02f, 0.21f);
			textures.last().scale = glm::vec2(0.46f, 0.44f);

			avatarTexture = textures.size();
			textures.emplace("textures/avatar rot.png");
			textures.last().translate = glm::vec2(0.02f, 0.16f);
			textures.last().scale = glm::vec2(0.29f, 0.32f);
			textures.last().darkToTransparent = true;

			recursiveFaceAnimationTexture = textures.size();
			textures.emplace("textures/recursive face animation.jpg");
			textures.last().minFilter = GL_LINEAR;
		}

		void loadAudio()
		{
			auto& musics = Globals::Components().musics();
			musics.emplace("audio/Ghosthack-Ambient Beds_Darkest Hour_Am 70Bpm (WET).ogg", 0.8f).play();

			auto& soundsBuffers = Globals::Components().soundsBuffers();
			missileExplosionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Impact - Detonate.wav").getComponentId();
			playerExplosionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack-AC21_Impact_Cracked.wav").getComponentId();
			missileLaunchingSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Whoosh - 5.wav", 0.2f).getComponentId();
			collisionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Impact - Edge.wav").getComponentId();
			thrustSoundBuffer = soundsBuffers.emplace("audio/thrust.wav", 0.2f).getComponentId();
			grappleSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Synth - Choatic_C.wav").getComponentId();
			avatarSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Scrape - Horror_C.wav", 0.4f).getComponentId();

			for (float x : {-40.0f, 40.0f})
				Tools::CreateAndPlaySound(avatarSoundBuffer, [x]() { return glm::vec2(x, -40.0f); },
					[](auto& sound) {
						sound.setLoop(true);
					});
		}

		void setAnimations()
		{
			for (auto& flameAnimatedTextureForPlayer: flameAnimatedTextureForPlayers)
			{
				flameAnimatedTextureForPlayer = Globals::Components().staticAnimatedTextures().size();
				Globals::Components().staticAnimatedTextures().add({ TCM::StaticTexture(flameAnimationTexture), { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
					AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal });
				Globals::Components().staticAnimatedTextures().last().start(true);
			}

			flameAnimatedTexture = Globals::Components().staticAnimatedTextures().size();
			Globals::Components().staticAnimatedTextures().add(Globals::Components().staticAnimatedTextures().last());

			invertedFlameAnimatedTexture = Globals::Components().staticAnimatedTextures().size();
			Globals::Components().staticAnimatedTextures().add(Globals::Components().staticAnimatedTextures().last());
			Globals::Components().staticAnimatedTextures().last().setAdditionalTransformation({ 0.0f, 0.0f }, glm::pi<float>());

			recursiveFaceAnimatedTexture = Globals::Components().staticAnimatedTextures().size();
			Globals::Components().staticAnimatedTextures().add({ TCM::StaticTexture(recursiveFaceAnimationTexture), { 263, 525 }, { 5, 10 }, { 0, 0 }, 210, 473, { 52, 52 }, 0.02f, 50, 0,
				AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal });
			Globals::Components().staticAnimatedTextures().last().start(true);
		}

		void createBackground()
		{
			Tools::CreateJuliaBackground([this]() {
				const auto averageCenter = std::accumulate(playersHandler.getPlayersHandlers().begin(), playersHandler.getPlayersHandlers().end(),
					glm::vec2(0.0f), [](const auto& acc, const auto& currentHandler) {
						return acc + Globals::Components().planes()[currentHandler.playerId].getOrigin2D();
					}) / (float)playersHandler.getPlayersHandlers().size();
				return averageCenter * 0.0001f; });
		}

		void createForeground()
		{
			Tools::CreateFogForeground(2, 0.02f, fogTexture, [&]() mutable {
				return glm::vec4(1.0f, 1.0f, 1.0f, smokeIntensity + 1.0f);
				});
		}

		Components::RenderingSetup createRecursiveFaceRS(std::function<glm::vec4()> colorF, glm::vec2 fadingRange) const
		{
			return { [=,
				colorUniform = Uniforms::Uniform4f(),
				visibilityReduction = Uniforms::Uniform1b(),
				fullVisibilityDistance = Uniforms::Uniform1f(),
				invisibilityDistance = Uniforms::Uniform1f(),
				colorF = std::move(colorF)
			] (Shaders::ProgramId program) mutable {
				if (!colorUniform.isValid())
				{
					colorUniform = Uniforms::Uniform4f(program, "color");
					visibilityReduction = Uniforms::Uniform1b(program, "visibilityReduction");
					fullVisibilityDistance = Uniforms::Uniform1f(program, "fullVisibilityDistance");
					invisibilityDistance = Uniforms::Uniform1f(program, "invisibilityDistance");
				}

				colorUniform(colorF());

				visibilityReduction(true);
				fullVisibilityDistance(fadingRange.x);
				invisibilityDistance(fadingRange.y);

				return [=]() mutable {
					colorUniform(Globals::Components().graphicsSettings().defaultColor);
					visibilityReduction(false);
					};
			} };
		}

		void createAdditionalDecorations() const
		{
			const auto blendingTexture = Globals::Components().staticBlendingTextures().size();
			Globals::Components().staticBlendingTextures().add({ TCM::StaticAnimatedTexture(invertedFlameAnimatedTexture), TCM::StaticTexture(ppTexture), TCM::StaticTexture(skullTexture), TCM::StaticTexture(avatarTexture) });

			for (int i = 0; i < 2; ++i)
			{
				glm::vec2 portraitCenter(i == 0 ? -40.0f : 40.0f, -40.0f);
				Globals::Components().renderingSetups().emplace([=,
					addBlendingColor = Uniforms::Uniform4f()
				](Shaders::ProgramId program) mutable {
					if (!addBlendingColor.isValid())
						addBlendingColor = Uniforms::Uniform4f(program, "addBlendingColor");

					float minDistance = std::numeric_limits<float>::max();
					for (const auto& playerHandler : playersHandler.getPlayersHandlers())
						minDistance = std::min(minDistance, glm::distance(Globals::Components().planes()[playerHandler.playerId].getOrigin2D(), portraitCenter));
					const float avatarOpacity = glm::min(0.0f, minDistance / 3.0f - 5.0f);

					i == 0
						? addBlendingColor({ 1.0f, smokeIntensity * 2.0f, avatarOpacity, 0.0f })
						: addBlendingColor({ 0.0f, smokeIntensity, 1.0f - smokeIntensity * 5.0f, 0.0f });

					return [=]() mutable {
						addBlendingColor({ 0.0f, 0.0f, 0.0f, 0.0f });
					};
				});

				Globals::Components().staticDecorations().emplace(Shapes2D::CreateVerticesOfRectangle(portraitCenter, { 10.0f, 10.0f }),
					TCM::StaticBlendingTexture(blendingTexture), Shapes2D::CreateTexCoordOfRectangle(), Globals::Components().renderingSetups().size() - 1,
					RenderLayer::NearMidground).preserveTextureRatio = true;
			}

			{
				Tools::CubicHermiteSpline spline({ { -5.0f, 5.0f }, { -5.0f, -5.0f }, { 5.0f, -5.0f }, { 5.0f, 5.0f }, { -5.0f, 5.0f }, {-5.0f, -5.0f}, { 5.0f, -5.0f } }, true, true);
				std::vector<glm::vec3> splineInterpolation;
				const int complexity = 100;
				splineInterpolation.reserve(complexity);
				for (int i = 0; i < complexity; ++i)
					splineInterpolation.push_back(glm::vec3(spline.getInterpolation((float)i / (complexity - 1)) + glm::vec2(0.0f, 30.0f), 0.0f));
				Globals::Components().staticDecorations().emplace(std::move(splineInterpolation));
				Globals::Components().staticDecorations().last().drawMode = GL_LINE_STRIP;
			}
		}

		void createMovableWalls()
		{
			Globals::Components().renderingSetups().emplace([
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
					Globals::Components().staticWalls().last().subsequence.emplace_back(Shapes2D::CreateVerticesOfLineOfRectangles({ 0.4f, 0.4f },
						{ { -0.5f, -5.0f }, { 0.5f, -5.0f }, { 0.5f, 5.0f }, { -0.5f, 5.0f }, { -0.5f, -5.0f } },
						{ 1.0f, 1.0f }, { 0.0f, glm::two_pi<float>() }, { 0.5f, 1.0f }), Shapes2D::CreateTexCoordOfRectangle(),
						TCM::StaticTexture(roseTexture));
					Globals::Components().staticWalls().last().subsequence.back().modelMatrixF =
						Globals::Components().staticWalls()[Globals::Components().staticWalls().size() - 1].modelMatrixF;
				};

				auto& wall1Body = *Globals::Components().staticWalls().emplace(
					Tools::CreateBoxBody({ 0.5f, 5.0f }, Tools::BodyParams().position({ 5.0f, -5.0f }).bodyType(b2_dynamicBody).density(0.2f)),
					TCM::StaticTexture(woodTexture), Globals::Components().renderingSetups().size() - 1, RenderLayer::NearMidground).body;
				wall1Body.GetFixtureList()->SetRestitution(0.5f);
				setRenderingSetupAndSubsequence();

				auto& wall2Body = *Globals::Components().staticWalls().emplace(
					Tools::CreateBoxBody({ 0.5f, 5.0f }, Tools::BodyParams().position({ 5.0f, 5.0f }).bodyType(b2_dynamicBody).density(0.2f)),
					TCM::StaticTexture(woodTexture), std::nullopt, RenderLayer::NearMidground).body;
				wall2Body.GetFixtureList()->SetRestitution(0.5f);
				setRenderingSetupAndSubsequence();

				Tools::CreateRevoluteJoint(wall1Body, wall2Body, { 5.0f, 0.0f });
			}

			const auto blendingTexture = Globals::Components().staticBlendingTextures().size();
			Globals::Components().staticBlendingTextures().add({ TCM::StaticTexture(fractalTexture), TCM::StaticTexture(woodTexture), TCM::StaticTexture(spaceRockTexture), TCM::StaticTexture(foiledEggsTexture) });

			for (const float pos : {-30.0f, 30.0f})
			{
				Globals::Components().staticWalls().emplace(Tools::CreateCircleBody(5.0f, Tools::BodyParams().position({ 0.0f, pos }).bodyType(b2_dynamicBody).density(0.01f)),
					TCM::StaticTexture(), Globals::Components().renderingSetups().size(), RenderLayer::Midground, Globals::Shaders().texturedColorThreshold().getProgramId());

				Globals::Components().renderingSetups().emplace([=, this, wallId = Globals::Components().staticWalls().size() - 1](auto) {
					Tools::MVPInitialization(Globals::Shaders().texturedColorThreshold(), Globals::Components().staticWalls()[wallId].modelMatrixF());

					if (pos < 0.0f)
					{
						Tools::PrepareTexturedRender(Globals::Shaders().texturedColorThreshold(), TCM::StaticTexture(orbTexture), true);
						Globals::Shaders().texturedColorThreshold().texturesCustomTransform(Tools::TextureTransform({ 0.0f, 0.0f }, 0.0f, { 5.0f, 5.0f }));
					}
					else
					{
						Tools::PrepareTexturedRender(Globals::Shaders().texturedColorThreshold(), TCM::StaticBlendingTexture(blendingTexture), true);
					}

					const float simulationDuration = Globals::Components().physics().simulationDuration;
					Globals::Shaders().texturedColorThreshold().invisibleColor({ 0.0f, 0.0f, 0.0f });
					Globals::Shaders().texturedColorThreshold().invisibleColorThreshold((-glm::cos(simulationDuration * 0.5f) + 1.0f) * 0.5f);
					return [=]() mutable { Globals::Shaders().texturedColorThreshold().texturesCustomTransform(glm::mat4(1.0f)); };
					});

				auto& wall = Globals::Components().staticWalls().emplace(Tools::CreateCircleBody(10.0f, Tools::BodyParams().position({ pos, 0.0f }).bodyType(b2_dynamicBody).density(0.01f)),
					TCM::StaticTexture(0));
				wall.renderLayer = RenderLayer::NearMidground;
				wall.renderF = []() { return false; };

				Globals::Components().renderingSetups().emplace([
						wallId = Globals::Components().staticWalls().size() - 1,
						texturedProgram = Shaders::Programs::TexturedAccessor()
					](Shaders::ProgramId program) mutable {
						if (!texturedProgram.isValid()) texturedProgram = program;
						texturedProgram.color(glm::vec4(
							glm::sin(Globals::Components().physics().simulationDuration* glm::two_pi<float>() * 0.2f) + 1.0f) / 2.0f);
						texturedProgram.model(Globals::Components().staticWalls()[wallId].modelMatrixF());
						return [=]() mutable { texturedProgram.color(Globals::Components().graphicsSettings().defaultColor); };
					});

				wall.subsequence.emplace_back(Shapes2D::CreateVerticesOfFunctionalRectangles({ 1.0f, 1.0f },
					[](float input) { return glm::vec2(glm::cos(input * 100.0f) * input * 10.0f, glm::sin(input * 100.0f) * input * 10.0f); },
					[](float input) { return glm::vec2(input + 0.3f, input + 0.3f); },
					[](float input) { return input * 600.0f; },
					[value = 0.0f]() mutable->std::optional<float> {
						if (value > 1.0f) return std::nullopt;
						float result = value;
						value += 0.002f;
						return result;
					}), Shapes2D::CreateTexCoordOfRectangle(), TCM::StaticTexture(roseTexture), Globals::Components().renderingSetups().size() - 1);
			}

			Globals::Components().staticWalls().last().resolutionMode = ResolutionMode::PixelArtBlend0;
			lowResBodies.insert(Globals::Components().staticWalls().last().body.get());
		}

		void createStationaryWalls()
		{
			const float levelWidthHSize = 80.0f;
			const float levelHeightHSize = 50.0f;
			const float bordersHGauge = 100.0f;

			auto renderingSetup = Globals::Components().renderingSetups().size();
			Globals::Components().renderingSetups().emplace([
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

			const auto blendingTexture = Globals::Components().staticBlendingTextures().size();
			Globals::Components().staticBlendingTextures().add({ TCM::StaticTexture(fractalTexture), TCM::StaticTexture(woodTexture), TCM::StaticTexture(spaceRockTexture), TCM::StaticTexture(foiledEggsTexture) });

			Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ bordersHGauge, levelHeightHSize + bordersHGauge * 2 },
				Tools::BodyParams().position({ -levelWidthHSize - bordersHGauge, 0.0f })), TCM::StaticBlendingTexture(blendingTexture), renderingSetup, RenderLayer::NearMidground).preserveTextureRatio = true;
			Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ bordersHGauge, levelHeightHSize + bordersHGauge * 2 },
				Tools::BodyParams().position({ levelWidthHSize + bordersHGauge, 0.0f })), TCM::StaticBlendingTexture(blendingTexture), renderingSetup, RenderLayer::NearMidground).preserveTextureRatio = true;
			Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ levelHeightHSize + bordersHGauge * 2, bordersHGauge },
				Tools::BodyParams().position({ 0.0f, -levelHeightHSize - bordersHGauge })), TCM::StaticBlendingTexture(blendingTexture), renderingSetup, RenderLayer::NearMidground).preserveTextureRatio = true;
			Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ levelHeightHSize + bordersHGauge * 2, bordersHGauge },
				Tools::BodyParams().position({ 0.0f, levelHeightHSize + bordersHGauge })), TCM::StaticBlendingTexture(blendingTexture), renderingSetup, RenderLayer::NearMidground).preserveTextureRatio = true;

			renderingSetup = Globals::Components().renderingSetups().size();
			Globals::Components().renderingSetups().emplace([
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

			Globals::Components().staticDecorations().emplace(Shapes2D::CreateVerticesOfLineOfRectangles({ 1.5f, 1.5f },
				{ { -levelWidthHSize, -levelHeightHSize }, { levelWidthHSize, -levelHeightHSize }, { levelWidthHSize, levelHeightHSize },
				{ -levelWidthHSize, levelHeightHSize }, { -levelWidthHSize, -levelHeightHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), TCM::StaticTexture(weedTexture), Shapes2D::CreateTexCoordOfRectangle(), renderingSetup);
			Globals::Components().staticDecorations().last().renderLayer = RenderLayer::FarForeground;
			//Globals::Components().decorations().back().resolutionMode = ResolutionMode::PixelArtBlend0;
		}

		void createGrapples() const
		{
			Globals::Components().grapples().emplace(Tools::CreateCircleBody(1.0f, Tools::BodyParams().position({ 0.0f, 10.0f })),
				TCM::StaticTexture(orbTexture)).influenceRadius = 15.0f;

			Globals::Components().renderingSetups().emplace([
				colorUniform = Uniforms::Uniform4f()
			](Shaders::ProgramId program) mutable {
					if (!colorUniform.isValid()) colorUniform = Uniforms::Uniform4f(program, "color");
					colorUniform(glm::vec4((glm::sin(Globals::Components().physics().simulationDuration / 3.0f * glm::two_pi<float>()) + 1.0f) / 2.0f));
					return [=]() mutable { colorUniform(Globals::Components().graphicsSettings().defaultColor); };
				});

			Globals::Components().grapples().emplace(Tools::CreateCircleBody(1.0f, Tools::BodyParams().position({ 0.0f, -10.0f })),
				TCM::StaticTexture(orbTexture), Globals::Components().renderingSetups().size() - 1).influenceRadius = 15.0f;

			Globals::Components().renderingSetups().emplace([
				texturesCustomTransformUniform = Uniforms::UniformMat4f()
			](Shaders::ProgramId program) mutable {
					if (!texturesCustomTransformUniform.isValid()) texturesCustomTransformUniform = Uniforms::UniformMat4f(program, "texturesCustomTransform");
					texturesCustomTransformUniform(Tools::TextureTransform({ 0.0f, 0.0f }, 0.0f, { 2.0f, 2.0f }));
					return [=]() mutable { texturesCustomTransformUniform(glm::mat4(1.0f)); };
				});

			Globals::Components().grapples().emplace(Tools::CreateCircleBody(2.0f,
				Tools::BodyParams().position({ -10.0f, -30.0f }).bodyType(b2_dynamicBody).density(0.1f).restitution(0.2f)),
				TCM::StaticTexture(orbTexture), Globals::Components().renderingSetups().size() - 1).influenceRadius = 30.0f;

			auto& grapple = Globals::Components().grapples().emplace(Tools::CreateCircleBody(4.0f,
				Tools::BodyParams().position({ -10.0f, 30.0f }).bodyType(b2_dynamicBody).density(0.1f).restitution(0.2f)), TCM::StaticTexture(0));
			grapple.influenceRadius = 30.0f;
			grapple.renderF = []() { return false; };
			grapple.subsequence.emplace_back(Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 5.2f, 5.2f }),
				Shapes2D::CreateTexCoordOfRectangle(), TCM::StaticAnimatedTexture(recursiveFaceAnimatedTexture));
			grapple.subsequence.back().modelMatrixF = grapple.modelMatrixF;
			grapple.subsequence.back().renderingSetup = Globals::Components().renderingSetups().size();
			Globals::Components().renderingSetups().add(createRecursiveFaceRS([]() { return glm::vec4(1.0f); }, {3.0f, 4.0f}));
		}

		void setCamera() const
		{
			playersHandler.setCamera(Tools::PlayersHandler::CameraParams().projectionHSizeMin([this]() { return projectionHSizeBase; }));
		}

		void setFramesRoutines()
		{
			Globals::Components().stepSetups().emplace([&]()
				{
					explosionFrame = false;
				});
		}

		void createSpawners()
		{
			const auto alpha = std::make_shared<float>(0.0f);
			const unsigned standardRS = Globals::Components().renderingSetups().size();
			Globals::Components().renderingSetups().emplace(
				[=, colorUniform = Uniforms::Uniform4f()](Shaders::ProgramId program) mutable {
					if (!colorUniform.isValid()) colorUniform = Uniforms::Uniform4f(program, "color");
					colorUniform(glm::vec4(*alpha));
					return [=]() mutable { colorUniform(Globals::Components().graphicsSettings().defaultColor); };
				});

			const unsigned recursiveFaceRS = Globals::Components().renderingSetups().size();
			Globals::Components().renderingSetups().add(createRecursiveFaceRS([=]() { return glm::vec4(*alpha); }, {1.0f, 2.0f}));

			auto spawner = [this, alpha, standardRS, recursiveFaceRS, first = true](float duration, auto& spawner) mutable -> bool // Type deduction doesn't get it is always bool.
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
					auto& wall = Globals::Components().dynamicWalls().emplace(Tools::CreateBoxBody({ 5.0f, 5.0f },
						Tools::BodyParams().position({ -50.0f, 30.0f })), TCM::StaticTexture(woodTexture, { 0.0f, 0.0f }, 0.0f, { 5.0f, 5.0f }), standardRS);
					dynamicWallId = wall.getComponentId();

					auto& grapple = Globals::Components().grapples().emplace(Tools::CreateCircleBody(2.0f, Tools::BodyParams().position({ 50.0f, 30.0f })),
						TCM::StaticAnimatedTexture(recursiveFaceAnimatedTexture, { 0.0f, 0.0f }, 0.0f, { 6.0f, 6.0f }), recursiveFaceRS);
					grapple.influenceRadius = 20.0f;
					dynamicGrappleId = grapple.getComponentId();
				}
				else if (duration >= existenceDuration)
				{
					first = true;
					Globals::Components().dynamicWalls()[dynamicWallId].state = ComponentState::Outdated;
					Globals::Components().grapples()[dynamicGrappleId].state = ComponentState::Outdated;
					Globals::Components().deferredActions().emplace([spawner](float duration) mutable { return spawner(duration, spawner); }, existenceDuration);

					return false;
				}

				return true;
			};
			Globals::Components().deferredActions().emplace([spawner](float duration) mutable { return spawner(duration, spawner); });
		}

		void initHandlers()
		{
			playersHandler.initPlayers(planeTextures, flameAnimatedTextureForPlayers, false,
				[](unsigned playerId, unsigned numOfPlayers) {
					const float gap = 5.0f;
					const float farPlayersDistance = gap * (numOfPlayers - 1);
					return glm::vec3(-10.0f, -farPlayersDistance / 2.0f + gap * playerId, 0.0f);
				}, false, thrustSoundBuffer, grappleSoundBuffer);

			missilesHandler.initCollisions();
			missilesHandler.setPlayersHandler(playersHandler);
			missilesHandler.setExplosionTexture(explosionTexture);
			missilesHandler.setMissileTexture(missile2Texture);
			missilesHandler.setFlameAnimatedTexture(flameAnimatedTexture);
			missilesHandler.setResolutionModeF([this](const auto& targetBody) {
				return lowResBodies.count(&targetBody) ? ResolutionMode::LowPixelArtBlend1 : ResolutionMode::LowestLinearBlend1;
				});
			missilesHandler.setExplosionF([this](auto pos) {
				Tools::CreateAndPlaySound(missileExplosionSoundBuffer, [pos]() { return pos; });
				explosionFrame = true;
				});
		}

		void collisionHandlers()
		{
			auto collisionSound = Tools::SkipDuplicatedBodiesCollisions([this](const auto& plane, const auto& obstacle) {
				Tools::CreateAndPlaySound(collisionSoundBuffer,
					[pos = *Tools::GetCollisionPoint(*plane.GetBody(), *obstacle.GetBody())]() {
						return pos;
					},
					[&](auto& sound) {
						sound.setVolume(std::sqrt(Tools::GetRelativeVelocity(*plane.GetBody(), *obstacle.GetBody()) / 20.0f));
						sound.setPitch(Tools::Random(0.9f, 1.5f));
					});
			});

			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::plane, Globals::CollisionBits::plane | Globals::CollisionBits::wall,
				collisionSound);
			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::wall, Globals::CollisionBits::wall,
				collisionSound);
		}

		void step()
		{
			playersHandler.gamepadsAutodetectionStep([](auto) { return glm::vec3(0.0f); });
			playersHandler.controlStep([this](unsigned playerHandlerId, bool fire) {
				missilesHandler.launchingMissile(playerHandlerId, fire, missileLaunchingSoundBuffer);
				});
		
			const auto& mouse = Globals::Components().mouse();
			const auto& gamepads = Globals::Components().gamepads();

			if (mouse.pressing.mmb || gamepads[0].pressing.rShoulder)
				Globals::Components().physics().gameSpeed = std::clamp(Globals::Components().physics().gameSpeed +
					(mouse.pressed.wheel + gamepads[0].pressed.dUp * 1 + gamepads[0].pressed.dDown * -1) * 0.1f, 0.0f, 2.0f);
			else
				projectionHSizeBase = std::clamp(projectionHSizeBase + (mouse.pressed.wheel +
					gamepads[0].pressed.dUp * 1 + gamepads[0].pressed.dDown * -1) * -5.0f, 5.0f, 100.0f);

			//textureAngle += Globals::Components().physics().frameDuration * 0.2f;

			smokeTargetIntensity += explosionFrame
				? Globals::Components().physics().frameDuration * 10.0f
				: -Globals::Components().physics().frameDuration * 0.2f;
			smokeTargetIntensity = std::clamp(smokeTargetIntensity, 0.0f, 1.0f);

			smokeIntensity += (smokeTargetIntensity - smokeIntensity) * Globals::Components().physics().frameDuration * 5.0f;
			smokeIntensity = std::clamp(smokeIntensity, 0.0f, 1.0f);
		}

	private:
		std::array<ComponentId, 4> planeTextures{ 0 };
		ComponentId spaceRockTexture = 0;
		ComponentId woodTexture = 0;
		ComponentId orbTexture = 0;
		ComponentId weedTexture = 0;
		ComponentId roseTexture = 0;
		ComponentId fogTexture = 0;
		ComponentId flameAnimationTexture = 0;
		ComponentId missile1Texture = 0;
		ComponentId missile2Texture = 0;
		ComponentId explosionTexture = 0;
		ComponentId foiledEggsTexture = 0;
		ComponentId fractalTexture = 0;
		ComponentId mosaicTexture = 0;
		ComponentId ppTexture = 0;
		ComponentId skullTexture = 0;
		ComponentId avatarTexture = 0;
		ComponentId recursiveFaceAnimationTexture = 0;

		std::array<ComponentId, 4> flameAnimatedTextureForPlayers{ 0 };
		ComponentId flameAnimatedTexture = 0;
		ComponentId invertedFlameAnimatedTexture = 0;
		ComponentId recursiveFaceAnimatedTexture = 0;

		ComponentId playerExplosionSoundBuffer = 0;
		ComponentId missileExplosionSoundBuffer = 0;
		ComponentId missileLaunchingSoundBuffer = 0;
		ComponentId collisionSoundBuffer = 0;
		ComponentId thrustSoundBuffer = 0;
		ComponentId grappleSoundBuffer = 0;
		ComponentId avatarSoundBuffer = 0;

		float projectionHSizeBase = 20.0f;

		bool explosionFrame = false;

		float smokeIntensity = 0.0f;
		float smokeTargetIntensity = 0.0f;

		float textureAngle = 0.0f;

		ComponentId dynamicWallId = 0;
		ComponentId dynamicGrappleId = 0;

		Tools::PlayersHandler playersHandler;
		Tools::MissilesHandler missilesHandler;

		std::unordered_set<const b2Body*> lowResBodies;
	};

	Playground::Playground():
		impl(std::make_unique<Impl>())
	{
		impl->globalSettings();
		impl->loadTextures();
		impl->loadAudio();
		impl->setAnimations();
		impl->createBackground();
		impl->createMovableWalls();
		impl->createStationaryWalls();
		impl->createGrapples();
		impl->createForeground();
		impl->createAdditionalDecorations();
		impl->setCamera();
		impl->setFramesRoutines();
		impl->createSpawners();
		impl->initHandlers();
		impl->collisionHandlers();
	}

	Playground::~Playground() = default;

	void Playground::step()
	{
		impl->step();
	}
}
