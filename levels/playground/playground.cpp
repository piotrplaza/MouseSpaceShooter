#include "playground.hpp"

#include <components/systemInfo.hpp>
#include <components/physics.hpp>
#include <components/plane.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/camera2D.hpp>
#include <components/camera3D.hpp>
#include <components/decoration.hpp>
#include <components/graphicsSettings.hpp>
#include <components/keyboard.hpp>
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

#include <ogl/uniformsUtils.hpp>
#include <ogl/shaders/texturedColorThreshold.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/shaders/trails.hpp>
#include <ogl/renderingHelpers.hpp>

#include <tools/Shapes2D.hpp>
#include <tools/Shapes3D.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>
#include <tools/playersHandler.hpp>
#include <tools/missilesHandler.hpp>
#include <tools/splines.hpp>
#include <tools/particleSystemHelpers.hpp>

#include <commonTypes/fTypes.hpp>

#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <optional>
#include <future>
#include <execution>

namespace Levels
{
	class Playground::Impl
	{
	public:
		void globalSettings() const
		{
			auto& graphicsSettings = Globals::Components().graphicsSettings();
			graphicsSettings.defaultColorF = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
			graphicsSettings.pointSize = 3.0f;

			Globals::Components().mainFramebufferRenderer().renderer = Tools::Demo3DRotatedFullscreenRenderer(Globals::Shaders().textured());
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().staticTextures();

			planeTextures[0] = textures.emplace("textures/plane 1.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.6f, 1.8f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			planeTextures[1] = textures.emplace("textures/alien ship 1.png");
			textures.last().translate = glm::vec2(-0.2f, 0.0f);
			textures.last().scale = glm::vec2(1.9f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			planeTextures[2] = textures.emplace("textures/plane 2.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.8f, 1.8f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			planeTextures[3] = textures.emplace("textures/alien ship 2.png");
			textures.last().translate = glm::vec2(0.0f, 0.0f);
			textures.last().scale = glm::vec2(1.45f, 1.4f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			spaceRockTexture = textures.size();
			textures.emplace("textures/space rock.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(5.0f);
			textures.last().preserveAspectRatio = true;

			woodTexture = textures.size();
			textures.emplace("textures/wood.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(5.0f);
			textures.last().preserveAspectRatio = true;

			orbTexture = textures.size();
			textures.emplace("textures/orb.png");
			textures.last().scale = glm::vec2(2.0f);
			textures.last().preserveAspectRatio = true;

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
			textures.last().preserveAspectRatio = true;

			missile2Texture = textures.size();
			textures.emplace("textures/missile 2.png");
			textures.last().minFilter = GL_LINEAR;
			textures.last().scale = glm::vec2(0.4f, 0.45f);
			textures.last().preserveAspectRatio = true;

			explosionTexture = textures.size();
			textures.emplace("textures/explosion.png");

			foiledEggsTexture = textures.size();
			textures.emplace("textures/foiled eggs.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(100.0f, 100.0f);
			textures.last().preserveAspectRatio = true;

			fractalTexture = textures.size();
			textures.emplace("textures/fractal.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(50.0f, 50.0f);
			textures.last().preserveAspectRatio = true;

			mosaicTexture = textures.size();
			textures.emplace("textures/mosaic.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(1.0f, 1.0f);

			ppTexture = textures.size();
			textures.emplace("textures/pp.png");
			textures.last().preserveAspectRatio = true;

			skullTexture = textures.size();
			textures.emplace("textures/skull rot.png");
			textures.last().translate = glm::vec2(0.02f, 0.21f);
			textures.last().scale = glm::vec2(0.46f, 0.44f);
			textures.last().preserveAspectRatio = true;

			avatarTexture = textures.size();
			textures.emplace(TextureFile("textures/avatar rot.png", 0, true, TextureFile::AdditionalConversion::DarkToTransparent));
			textures.last().translate = glm::vec2(0.02f, 0.16f);
			textures.last().scale = glm::vec2(0.29f, 0.32f);
			textures.last().preserveAspectRatio = true;

			recursiveFaceAnimationTexture = textures.size();
			textures.emplace("textures/recursive face animation.jpg");
			textures.last().minFilter = GL_LINEAR;

			marbleTexture = textures.emplace("textures/green marble.jpg").getComponentId();
			textures.last().wrapMode = GL_MIRRORED_REPEAT;
		}

		void loadAudio()
		{
			auto& musics = Globals::Components().staticMusics();
			musics.emplace("audio/Ghosthack-Ambient Beds_Darkest Hour_Am 70Bpm (WET).ogg", 0.8f).play();

			auto& soundsBuffers = Globals::Components().staticSoundsBuffers();
			missileExplosionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Impact - Detonate.wav").getComponentId();
			playerExplosionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack-AC21_Impact_Cracked.wav").getComponentId();
			missileLaunchingSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Whoosh - 5.wav", 0.2f).getComponentId();
			collisionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Impact - Edge.wav").getComponentId();
			thrustSoundBuffer = soundsBuffers.emplace("audio/thrust.wav", 0.2f).getComponentId();
			grappleSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Synth - Choatic_C.wav").getComponentId();
			avatarSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Scrape - Horror_C.wav", 0.4f).getComponentId();

			for (float x : {-40.0f, 40.0f})
				Tools::CreateAndPlaySound(CM::SoundBuffer(avatarSoundBuffer, true), [x]() { return glm::vec2(x, -40.0f); },
					[](auto& sound) {
						sound.setLooping(true);
					});
		}

		void setAnimations()
		{
			for (auto& flameAnimatedTextureForPlayer: flameAnimatedTextureForPlayers)
			{
				flameAnimatedTextureForPlayer = Globals::Components().staticAnimatedTextures().add({ CM::Texture(flameAnimationTexture, true), { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
					AnimationData::Direction::Backward, AnimationData::Mode::Repeat, AnimationData::TextureLayout::Horizontal });
				Globals::Components().staticAnimatedTextures().last().start(true);
			}

			flameAnimatedTexture = Globals::Components().staticAnimatedTextures().size();
			Globals::Components().staticAnimatedTextures().add(Globals::Components().staticAnimatedTextures().last());

			invertedFlameAnimatedTexture = Globals::Components().staticAnimatedTextures().size();
			Globals::Components().staticAnimatedTextures().add(Globals::Components().staticAnimatedTextures().last());
			Globals::Components().staticAnimatedTextures().last().setAdditionalTransformation({ 0.0f, 0.0f }, glm::pi<float>());

			recursiveFaceAnimatedTexture = Globals::Components().staticAnimatedTextures().size();
			Globals::Components().staticAnimatedTextures().add({ CM::Texture(recursiveFaceAnimationTexture, true), { 263, 525 }, { 5, 10 }, { 0, 0 }, 210, 473, { 52, 52 }, 0.02f, 50, 0,
				AnimationData::Direction::Forward, AnimationData::Mode::Repeat, AnimationData::TextureLayout::Horizontal });
			Globals::Components().staticAnimatedTextures().last().start(true);
		}

		void createBackground()
		{
			Tools::CreateJuliaBackground(Tools::JuliaParams{}.juliaCOffsetF([this]() {
				const auto averageCenter = std::accumulate(playersHandler.getPlayersHandlers().begin(), playersHandler.getPlayersHandlers().end(),
					glm::vec2(0.0f), [](const auto& acc, const auto& currentHandler) {
						return acc + Globals::Components().planes()[currentHandler.playerId].getOrigin2D();
					}) / (float)playersHandler.getPlayersHandlers().size();
				return averageCenter * 0.0001f; }));

			{
				const int numOfCrosses = 10000;
				const int numOfLights = 40;

				const auto& physics = Globals::Components().physics();
				auto& staticDecorations = Globals::Components().staticDecorations();
				auto& dynamicDecorations = Globals::Components().decorations();

				{
					Tools::Shapes3D::AddCross(dynamicDecorations.emplace(), { 0.1f, 0.5f, 0.1f }, { 0.35f, 0.1f, 0.1f }, 0.15f, [](auto, glm::vec3 p) { return glm::vec2(p.x + p.z, p.y + p.z); });
					dynamicDecorations.last().params3D->ambient(0.4f).diffuse(0.8f).specular(0.8f).specularMaterialColorFactor(0.2f).lightModelEnabled(true).gpuSideInstancedNormalTransforms(true);
					dynamicDecorations.last().texture = CM::Texture(marbleTexture, true);
					dynamicDecorations.last().bufferDataUsage = GL_DYNAMIC_DRAW;
					dynamicDecorations.last().instancing.emplace().init(numOfCrosses, glm::mat4(1.0f));
					dynamicDecorations.last().renderLayer = RenderLayer::NearBackground;
					crossesId = dynamicDecorations.last().getComponentId();
				}

				for (unsigned i = 0; i < numOfLights; ++i)
				{
					auto& lights3D = Globals::Components().lights3D();
					lights3D.emplace(glm::vec3(0.0f), glm::vec3(1.0f), 0.6f, 1.0f);
					lights3D.last().stepF = ([&light = lights3D.last(), &crosses = dynamicDecorations[crossesId]]() { light.setEnabled(crosses.isEnabled()); });
				}

				for (const auto& light : Globals::Components().lights3D())
				{
					Tools::Shapes3D::AddSphere(staticDecorations.emplace(), 0.2f, 2, 3);
					staticDecorations.last().colorF = [&]() { return glm::vec4(light.color, 1.0f) + Globals::Components().graphicsSettings().backgroundColorF() * light.darkColorFactor; };
					staticDecorations.last().params3D->lightModelEnabled(false);
					staticDecorations.last().modelMatrixF = [&]() { return glm::rotate(glm::translate(glm::mat4(1.0f), light.position), physics.simulationDuration * 4.0f, { 1.0f, 1.0f, 1.0f }); };
					staticDecorations.last().stepF = ([&lightSphere = staticDecorations.last(), &crosses = dynamicDecorations[crossesId]]() { lightSphere.setEnabled(crosses.isEnabled()); });
					staticDecorations.last().renderLayer = RenderLayer::NearBackground;
				}
			}
		}

		void createForeground()
		{
			Tools::CreateFogForeground(2, 0.02f, CM::Texture(fogTexture, true), [&]() mutable {
				return glm::vec4(1.0f, 1.0f, 1.0f, smokeIntensity + 1.0f);
				});
		}

		RenderableDef::RenderingSetupF createRecursiveFaceRS(FVec4 colorF, glm::vec2 fadingRange) const
		{
			return { [=,
				colorUniform = UniformsUtils::Uniform4f(),
				visibilityReduction = UniformsUtils::Uniform1b(),
				fullVisibilityDistance = UniformsUtils::Uniform1f(),
				invisibilityDistance = UniformsUtils::Uniform1f(),
				colorF = std::move(colorF)
			] (ShadersUtils::ProgramId program) mutable {
				if (!colorUniform.isValid())
				{
					colorUniform.reset(program, "color");
					visibilityReduction.reset(program, "visibilityReduction");
					fullVisibilityDistance.reset(program, "fullVisibilityDistance");
					invisibilityDistance.reset(program, "invisibilityDistance");
				}

				colorUniform(colorF());

				visibilityReduction(true);
				fullVisibilityDistance(fadingRange.x);
				invisibilityDistance(fadingRange.y);

				return [=]() mutable {
					colorUniform(Globals::Components().graphicsSettings().defaultColorF());
					visibilityReduction(false);
				};
			} };
		}

		void createAdditionalDecorations() const
		{
			const auto blendingTexture = Globals::Components().staticBlendingTextures().size();
			Globals::Components().staticBlendingTextures().add({ CM::AnimatedTexture(invertedFlameAnimatedTexture, true), CM::Texture(ppTexture, true), CM::Texture(skullTexture, true), CM::Texture(avatarTexture, true) });

			for (int i = 0; i < 2; ++i)
			{
				glm::vec2 portraitCenter(i == 0 ? -40.0f : 40.0f, -40.0f);
				auto renderingSetupF = [=,
					addBlendingColor = UniformsUtils::Uniform4f()
				](ShadersUtils::ProgramId program) mutable {
					if (!addBlendingColor.isValid())
						addBlendingColor.reset(program, "addBlendingColor");

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
				};

				Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle(portraitCenter, { 10.0f, 10.0f }),
					CM::BlendingTexture(blendingTexture, true), Tools::Shapes2D::CreateTexCoordOfRectangle(), std::move(renderingSetupF),
					RenderLayer::NearMidground);
			}

			{
				Tools::CubicHermiteSpline spline({ { -5.0f, 5.0f }, { -5.0f, -5.0f }, { 5.0f, -5.0f }, { 5.0f, 5.0f }, { -5.0f, 5.0f }, {-5.0f, -5.0f}, { 5.0f, -5.0f } }, true, true);
				std::vector<glm::vec3> splineInterpolation;
				const int complexity = 100;
				splineInterpolation.reserve(complexity);
				for (int i = 0; i < complexity; ++i)
					splineInterpolation.push_back(glm::vec3(spline.getSplineSample((float)i / (complexity - 1)) + glm::vec2(0.0f, 30.0f), 0.0f));
				Globals::Components().staticDecorations().emplace(std::move(splineInterpolation));
				Globals::Components().staticDecorations().last().drawMode = GL_LINE_STRIP;
			}
		}

		void createMovableWalls()
		{
			auto renderingSetupF = [
				texturesCustomTransformUniform = UniformsUtils::UniformMat4f()
			](ShadersUtils::ProgramId program) mutable {
					if (!texturesCustomTransformUniform.isValid())
						texturesCustomTransformUniform.reset(program, "texturesCustomTransform");
					const float simulationDuration = Globals::Components().physics().simulationDuration;
					texturesCustomTransformUniform(Tools::TextureTransform(glm::vec2(glm::cos(simulationDuration), glm::sin(simulationDuration)) * 0.1f ));
					return [=]() mutable { texturesCustomTransformUniform(glm::mat4(1.0f)); };
				};

			{
				auto setRenderingSetupAndSubsequence = [&]()
				{
					Globals::Components().staticWalls().last().subsequence.emplace_back(Tools::Shapes2D::CreatePositionsOfLineOfRectangles({ 0.4f, 0.4f },
						{ { -0.5f, -5.0f }, { 0.5f, -5.0f }, { 0.5f, 5.0f }, { -0.5f, 5.0f }, { -0.5f, -5.0f } },
						{ 1.0f, 1.0f }, { 0.0f, glm::two_pi<float>() }, { 0.5f, 1.0f }), Tools::Shapes2D::CreateTexCoordOfRectangle(),
						CM::Texture(roseTexture, true));
					Globals::Components().staticWalls().last().subsequence.back().modelMatrixF =
						Globals::Components().staticWalls()[Globals::Components().staticWalls().size() - 1].modelMatrixF;
				};

				auto& wall1Body = *Globals::Components().staticWalls().emplace(
					Tools::CreateBoxBody({ 0.5f, 5.0f }, Tools::BodyParams().position({ 5.0f, -5.0f }).bodyType(b2_dynamicBody).density(0.2f)),
					CM::Texture(woodTexture, true), std::move(renderingSetupF), RenderLayer::NearMidground).body;
				wall1Body.GetFixtureList()->SetRestitution(0.5f);
				setRenderingSetupAndSubsequence();

				auto& wall2Body = *Globals::Components().staticWalls().emplace(
					Tools::CreateBoxBody({ 0.5f, 5.0f }, Tools::BodyParams().position({ 5.0f, 5.0f }).bodyType(b2_dynamicBody).density(0.2f)),
					CM::Texture(woodTexture, true), nullptr, RenderLayer::NearMidground).body;
				wall2Body.GetFixtureList()->SetRestitution(0.5f);
				setRenderingSetupAndSubsequence();

				Tools::CreateRevoluteJoint(wall1Body, wall2Body, { 5.0f, 0.0f });
			}

			const auto blendingTexture = Globals::Components().staticBlendingTextures().size();
			Globals::Components().staticBlendingTextures().add({ CM::Texture(fractalTexture, true), CM::Texture(woodTexture, true), CM::Texture(spaceRockTexture, true), CM::Texture(foiledEggsTexture, true) });

			for (const float pos : {-30.0f, 30.0f})
			{
				{
					auto renderingSetupF = [=, this, wallId = Globals::Components().staticWalls().size()](auto&) {
						Tools::MVPInitialization(Globals::Shaders().texturedColorThreshold(), Globals::Components().staticWalls()[wallId].modelMatrixF());

						if (pos < 0.0f)
						{
							Tools::PrepareTexturedRender(Globals::Shaders().texturedColorThreshold(), CM::Texture(orbTexture, true));
							Globals::Shaders().texturedColorThreshold().texturesCustomTransform(Tools::TextureTransform({ 0.0f, 0.0f }, 0.0f, { 5.0f, 5.0f }));
						}
						else
						{
							Tools::PrepareTexturedRender(Globals::Shaders().texturedColorThreshold(), CM::BlendingTexture(blendingTexture, true));
						}

						const float simulationDuration = Globals::Components().physics().simulationDuration;
						Globals::Shaders().texturedColorThreshold().invisibleColor({ 0.0f, 0.0f, 0.0f });
						Globals::Shaders().texturedColorThreshold().invisibleColorThreshold((-glm::cos(simulationDuration * 0.5f) + 1.0f) * 0.5f);
						return [=]() mutable { Globals::Shaders().texturedColorThreshold().texturesCustomTransform(glm::mat4(1.0f)); };
						};

					const auto radius = 5.0f;
					auto& disc = Globals::Components().staticWalls().emplace(Tools::CreateDiscBody(radius, Tools::BodyParams().position({ 0.0f, pos }).bodyType(b2_dynamicBody).density(0.01f)),
						CM::DummyTexture(), std::move(renderingSetupF), RenderLayer::Midground, &Globals::Shaders().texturedColorThreshold());

					constexpr int particleEmittersCount = 8;
					constexpr float radialStep = glm::two_pi<float>() / particleEmittersCount;
					for (int i = 0; i < particleEmittersCount; ++i)
					{
						const float angle = radialStep * i;
						auto finalAngleF = [=, &disc]() { return disc.getAngle() + angle; };
						Tools::CreateParticleSystem(Tools::ParticleSystemParams{}
							.position([=, &disc]() { return glm::vec3(disc.getOrigin2D() + glm::vec2(glm::cos(finalAngleF()), glm::sin(finalAngleF())) * radius, 0.0f); })
							.velocityFactor(1.0f).initVelocity([=]() { return glm::vec3(glm::vec2(glm::cos(finalAngleF()), glm::sin(finalAngleF())) * 10.0f, 0.0f); })
							.velocityOffset([&]() { return glm::vec3(disc.getVelocity(), 0.0f); })
							.globalForce(glm::vec3(0.0f))
							.particlesCount(10000));
					}
				}

				auto& wall = Globals::Components().staticWalls().emplace(Tools::CreateDiscBody(10.0f, Tools::BodyParams().position({ pos, 0.0f }).bodyType(b2_dynamicBody).density(0.01f)),
					CM::DummyTexture());
				wall.renderLayer = RenderLayer::NearMidground;
				wall.renderF = []() { return false; };

				auto renderingSetupF = [
						wallId = Globals::Components().staticWalls().size() - 1
					](ShadersUtils::AccessorBase& shaderBase) mutable {
						auto& program = static_cast<ShadersUtils::Programs::TexturedAccessor&>(shaderBase);
						program.color(glm::vec4(
							glm::sin(Globals::Components().physics().simulationDuration* glm::two_pi<float>() * 0.2f) + 1.0f) / 2.0f);
						program.model(Globals::Components().staticWalls()[wallId].modelMatrixF());
						return [&]() mutable { program.color(Globals::Components().graphicsSettings().defaultColorF()); };
					};

				wall.subsequence.emplace_back(Tools::Shapes2D::CreatePositionsOfFunctionalRectangles({ 1.0f, 1.0f },
					[](float input) { return glm::vec2(glm::cos(input * 100.0f) * input * 10.0f, glm::sin(input * 100.0f) * input * 10.0f); },
					[](float input) { return glm::vec2(input + 0.3f, input + 0.3f); },
					[](float input) { return input * 600.0f; },
					[value = 0.0f]() mutable->std::optional<float> {
						if (value > 1.0f) return std::nullopt;
						float result = value;
						value += 0.002f;
						return result;
					}), Tools::Shapes2D::CreateTexCoordOfRectangle(), CM::Texture(roseTexture, true), std::move(renderingSetupF));
			}

			Globals::Components().staticWalls().last().resolutionMode = { ResolutionMode::Resolution::H68 };
			lowResBodies.insert(Globals::Components().staticWalls().last().body.get());
		}

		void createStationaryWalls()
		{
			const float levelWidthHSize = 80.0f;
			const float levelHeightHSize = 50.0f;
			const float bordersHGauge = 100.0f;

			{
				auto renderingSetupF = [
					alphaFromBlendingTextureUniform = UniformsUtils::Uniform1b(),
						colorAccumulationUniform = UniformsUtils::Uniform1b(),
						texturesCustomTransform = UniformsUtils::UniformMat4fv<5>(),
						sceneCoordTextures = UniformsUtils::Uniform1b(),
						this
				](ShadersUtils::ProgramId program) mutable {
					if (!alphaFromBlendingTextureUniform.isValid())
						alphaFromBlendingTextureUniform.reset(program, "alphaFromBlendingTexture");
					if (!colorAccumulationUniform.isValid())
						colorAccumulationUniform.reset(program, "colorAccumulation");
					if (!texturesCustomTransform.isValid())
						texturesCustomTransform.reset(program, "texturesCustomTransform");
					if (!sceneCoordTextures.isValid())
						sceneCoordTextures.reset(program, "sceneCoordTextures");

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
					};

					const auto blendingTexture = Globals::Components().staticBlendingTextures().size();
					Globals::Components().staticBlendingTextures().add({ CM::Texture(fractalTexture, true), CM::Texture(woodTexture, true), CM::Texture(spaceRockTexture, true), CM::Texture(foiledEggsTexture, true) });

					Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ bordersHGauge, levelHeightHSize + bordersHGauge * 2 },
						Tools::BodyParams().position({ -levelWidthHSize - bordersHGauge, 0.0f })), CM::BlendingTexture(blendingTexture, true), renderingSetupF, RenderLayer::NearMidground);
					Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ bordersHGauge, levelHeightHSize + bordersHGauge * 2 },
						Tools::BodyParams().position({ levelWidthHSize + bordersHGauge, 0.0f })), CM::BlendingTexture(blendingTexture, true), renderingSetupF, RenderLayer::NearMidground);
					Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ levelHeightHSize + bordersHGauge * 2, bordersHGauge },
						Tools::BodyParams().position({ 0.0f, -levelHeightHSize - bordersHGauge })), CM::BlendingTexture(blendingTexture, true), renderingSetupF, RenderLayer::NearMidground);
					Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ levelHeightHSize + bordersHGauge * 2, bordersHGauge },
						Tools::BodyParams().position({ 0.0f, levelHeightHSize + bordersHGauge })), CM::BlendingTexture(blendingTexture, true), std::move(renderingSetupF), RenderLayer::NearMidground);
			}

			auto renderingSetupF = [
				playerUnhidingRadiusUniform = UniformsUtils::Uniform1f(),
				this
			](ShadersUtils::ProgramId program) mutable {
					if (!playerUnhidingRadiusUniform.isValid())
						playerUnhidingRadiusUniform.reset(program, "playerUnhidingRadius");

					playerUnhidingRadiusUniform(20.0f);

					return [=]() mutable
					{
						playerUnhidingRadiusUniform(0.0f);
					};
				};

			Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfLineOfRectangles({ 1.5f, 1.5f },
				{ { -levelWidthHSize, -levelHeightHSize }, { levelWidthHSize, -levelHeightHSize }, { levelWidthHSize, levelHeightHSize },
				{ -levelWidthHSize, levelHeightHSize }, { -levelWidthHSize, -levelHeightHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), CM::Texture(weedTexture, true), Tools::Shapes2D::CreateTexCoordOfRectangle(), std::move(renderingSetupF));
			Globals::Components().staticDecorations().last().renderLayer = RenderLayer::FarForeground;
			//Globals::Components().decorations().back().resolutionMode = ResolutionMode::PixelArtBlend0;
		}

		void createGrapples() const
		{
			Globals::Components().grapples().emplace(Tools::CreateDiscBody(1.0f, Tools::BodyParams().position({ 0.0f, 10.0f })),
				CM::Texture(orbTexture, true)).range = 15.0f;

			{
				auto renderingSetupF = [colorUniform = UniformsUtils::Uniform4f()](ShadersUtils::ProgramId program) mutable {
					if (!colorUniform.isValid())
						colorUniform.reset(program, "color");
					colorUniform(glm::vec4((glm::sin(Globals::Components().physics().simulationDuration / 3.0f * glm::two_pi<float>()) + 1.0f) / 2.0f));
					return [=]() mutable { colorUniform(Globals::Components().graphicsSettings().defaultColorF()); };
				};

				Globals::Components().grapples().emplace(Tools::CreateDiscBody(1.0f, Tools::BodyParams().position({ 0.0f, -10.0f })),
					CM::Texture(orbTexture, true), std::move(renderingSetupF)).range = 15.0f;
			}

			{
				auto renderingSetupF = [
					texturesCustomTransformUniform = UniformsUtils::UniformMat4f()
				](ShadersUtils::ProgramId program) mutable {
					if (!texturesCustomTransformUniform.isValid())
						texturesCustomTransformUniform.reset(program, "texturesCustomTransform");
					texturesCustomTransformUniform(Tools::TextureTransform({ 0.0f, 0.0f }, 0.0f, { 2.0f, 2.0f }));
					return [=]() mutable { texturesCustomTransformUniform(glm::mat4(1.0f)); };
					};

					Globals::Components().grapples().emplace(Tools::CreateDiscBody(2.0f,
						Tools::BodyParams().position({ -10.0f, -30.0f }).bodyType(b2_dynamicBody).density(0.1f).restitution(0.2f)),
						CM::Texture(orbTexture, true), std::move(renderingSetupF)).range = 30.0f;
			}

			auto& grapple = Globals::Components().grapples().emplace(Tools::CreateDiscBody(4.0f,
				Tools::BodyParams().position({ -10.0f, 30.0f }).bodyType(b2_dynamicBody).density(0.1f).restitution(0.2f)), CM::DummyTexture());
			grapple.range = 30.0f;
			grapple.renderF = []() { return false; };
			grapple.subsequence.emplace_back(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 5.2f, 5.2f }),
				Tools::Shapes2D::CreateTexCoordOfRectangle(), CM::AnimatedTexture(recursiveFaceAnimatedTexture, true));
			grapple.subsequence.back().modelMatrixF = grapple.modelMatrixF;
			grapple.subsequence.back().renderingSetupF = createRecursiveFaceRS([]() { return glm::vec4(1.0f); }, { 3.0f, 4.0f });
		}

		void setCamera()
		{
			playersHandler.setCamera(Tools::PlayersHandler::CameraParams().projectionHSizeMin([this]() { return projectionHSizeBase; }));
		}

		void setFramesRoutines()
		{
			Globals::Components().stepSetups().emplace([&]() { explosionFrame = false; return true; });
		}

		void createSpawners()
		{
			const auto alpha = std::make_shared<float>(0.0f);
			auto standardRSF =  [=, colorUniform = UniformsUtils::Uniform4f()](ShadersUtils::ProgramId program) mutable {
					if (!colorUniform.isValid())
						colorUniform.reset(program, "color");
					colorUniform(glm::vec4(*alpha));
					return [=]() mutable { colorUniform(Globals::Components().graphicsSettings().defaultColorF()); };
				};

			auto recursiveFaceRSF = createRecursiveFaceRS([=]() { return glm::vec4(*alpha); }, {1.0f, 2.0f});

			auto spawner = [this, alpha, standardRSF = std::move(standardRSF), recursiveFaceRSF = std::move(recursiveFaceRSF), first = true](float duration, auto& spawner) mutable -> bool // Type deduction doesn't get it is always bool.
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
					auto& wall = Globals::Components().walls().emplace(Tools::CreateBoxBody({ 5.0f, 5.0f },
						Tools::BodyParams().position({ -50.0f, 30.0f })), CM::Texture(woodTexture, true, { 0.0f, 0.0f }, 0.0f, { 5.0f, 5.0f }), standardRSF);
					dynamicWallId = wall.getComponentId();

					auto& grapple = Globals::Components().grapples().emplace(Tools::CreateDiscBody(2.0f, Tools::BodyParams().position({ 50.0f, 30.0f })),
						CM::AnimatedTexture(recursiveFaceAnimatedTexture, true, { 0.0f, 0.0f }, 0.0f, { 6.0f, 6.0f }), recursiveFaceRSF);
					grapple.range = 20.0f;
					dynamicGrappleId = grapple.getComponentId();
				}
				else if (duration >= existenceDuration)
				{
					first = true;
					Globals::Components().walls()[dynamicWallId].state = ComponentState::Outdated;
					Globals::Components().grapples()[dynamicGrappleId].state = ComponentState::Outdated;
					Globals::Components().deferredActions().emplace([spawner](float duration) mutable { return spawner(duration, spawner); }, existenceDuration);

					return false;
				}

				return true;
			};
			Globals::Components().deferredActions().emplace([spawner = std::move(spawner)](float duration) mutable { return spawner(duration, spawner); });
		}

		void initHandlers()
		{
			playersHandler.initPlayers(planeTextures, flameAnimatedTextureForPlayers, false,
				[](unsigned playerId, unsigned numOfPlayers) {
					const float gap = 5.0f;
					const float farPlayersDistance = gap * (numOfPlayers - 1);
					return glm::vec3(-10.0f, -farPlayersDistance / 2.0f + gap * playerId, 0.0f);
				}, false, CM::SoundBuffer(thrustSoundBuffer, true), CM::SoundBuffer(grappleSoundBuffer, true));

			missilesHandler.initCollisions();
			missilesHandler.setPlayersHandler(playersHandler);
			missilesHandler.setExplosionTexture(CM::Texture(explosionTexture, true));
			missilesHandler.setMissileTexture(CM::Texture(missile2Texture, true));
			missilesHandler.setFlameAnimatedTexture(CM::AnimatedTexture(flameAnimatedTexture, true));
			missilesHandler.setResolutionModeF([this](const auto& targetBody) {
				return lowResBodies.contains(&targetBody)
					? ResolutionMode{ ResolutionMode::Resolution::H135, ResolutionMode::Scaling::Nearest, ResolutionMode::Blending::Additive }
					: ResolutionMode{ ResolutionMode::Resolution::QuarterNative, ResolutionMode::Scaling::Linear, ResolutionMode::Blending::Additive};
				});
			missilesHandler.setExplosionF([this](auto pos) {
				Tools::CreateSparking(Tools::SparkingParams{}.sourcePoint(pos).initVelocity({ 50.0f, 0.0f }).sparksCount(1000).lineWidth(2.0f));
				Tools::CreateAndPlaySound(CM::SoundBuffer(missileExplosionSoundBuffer, true), [pos]() { return pos; });
				explosionFrame = true;
			});
		}

		void collisionHandlers()
		{
			auto collisionAction = Tools::SkipDuplicatedBodiesCollisions([this](const auto& plane, const auto& obstacle) {
				const auto collisionPoint = *Tools::GetCollisionPoint(*plane.GetBody(), *obstacle.GetBody());
				const auto relativeVelocity = Tools::GetRelativeVelocity(*plane.GetBody(), *obstacle.GetBody());
				const float relativeSpeed = glm::length(relativeVelocity);

				if (relativeSpeed > 10.0f)
					Tools::CreateSparking(Tools::SparkingParams{}.sourcePoint(collisionPoint).initVelocity({ relativeSpeed * 0.5f, 0.0f }).sparksCount((int)(10 * relativeSpeed)).lineWidth(4.0f));

				Tools::CreateAndPlaySound(CM::SoundBuffer(collisionSoundBuffer, true),
					[collisionPoint]() {
						return collisionPoint;
					},
					[&](auto& sound) {
						sound.setVolume(std::sqrt(Tools::GetRelativeSpeed(*plane.GetBody(), *obstacle.GetBody()) / 20.0f));
						sound.setPitch(Tools::RandomFloat(0.4f, 0.6f));
					});
			});

			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::actor, Globals::CollisionBits::actor | Globals::CollisionBits::wall,
				collisionAction);
			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::wall, Globals::CollisionBits::wall,
				collisionAction);
		}

		void gameStep()
		{
			playersHandler.gamepadsAutodetectionStep([](auto) { return glm::vec3(0.0f); });
			playersHandler.controlStep([this](unsigned playerHandlerId, bool fire) {
				missilesHandler.launchingMissile(playerHandlerId, fire, CM::SoundBuffer(missileLaunchingSoundBuffer, true));
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

		void decorationStep()
		{
			{
				const auto& physics = Globals::Components().physics();

				int i = 0;
				for (auto& light : Globals::Components().lights3D())
				{
					const float rotationSpeed = (-0.1f - (i * 0.03f)) * (i % 2 * 2.0f - 1.0f);
					const float radius = 1.0f + rotationSpeed * 5.0f;
					const glm::vec3 changeColorSpeed(1.0f, 0.6f, 0.3f);

					light.position = glm::vec3(glm::cos(physics.simulationDuration * rotationSpeed) * radius, glm::cos(physics.simulationDuration * rotationSpeed * 0.3f) * radius, glm::sin(physics.simulationDuration * rotationSpeed * 0.6f) * radius);
					light.color = { (glm::cos(physics.simulationDuration * changeColorSpeed.r * rotationSpeed) + 1.0f) / 2.0f,
						(glm::cos(physics.simulationDuration * changeColorSpeed.g * rotationSpeed) + 1.0f) / 2.0f,
						(glm::cos(physics.simulationDuration * changeColorSpeed.b * rotationSpeed) + 1.0f) / 2.0f };
					//light.color = glm::vec3(1.0f);
					++i;
				}
			}

			const auto& keyboard = Globals::Components().keyboard();
			auto& crosses = Globals::Components().decorations()[crossesId];

			if (keyboard.pressed['C'])
			{
				crosses.setEnabled(!crosses.isEnabled());
			}

			if (crosses.isEnabled())
			{
				const float transformSpeed = 0.00001f;
				const float transformBaseStep = 0.001f;

				const auto& physics = Globals::Components().physics();
				auto& transforms = crosses.instancing->transforms_;

				if (keyboard.pressed[0x26/*VK_UP*/])
					transformBase += transformBaseStep;
				if (keyboard.pressed[0x28/*VK_DOWN*/])
					transformBase -= transformBaseStep;
#if 1
				if (transformFuture.valid())
				{
					transformFuture.get();
					crosses.state = ComponentState::Changed;
				}

				transformFuture = std::async(std::launch::async, [=, simulationDuration = physics.simulationDuration, &transforms]() {
					Tools::ItToId itToId(transforms.size());
					std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), [=, &transforms](const auto i) {
						transforms[i] = glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0.0f, 1.0f, 0.0f })
							* glm::rotate(glm::mat4(1.0f), i * glm::pi<float>() * 0.001f, { 1.0f, 0.0f, 0.0f })
							* glm::rotate(glm::mat4(1.0f), i * glm::pi<float>() * 0.03f, { 0.0f, 1.0f, 0.0f })
							* glm::rotate(glm::mat4(1.0f), i * glm::pi<float>() * (transformBase - simulationDuration * transformSpeed), { 0.0f, 0.0f, 1.0f })
							* glm::translate(glm::mat4(1.0f), { i * 0.0005f, i * 0.0007f, i * 0.0009f });
						});
					});
#else
				Tools::ItToId itToId(transforms.size());
				std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), [&](const auto i) {
					transforms[i] = glm::rotate(glm::mat4(1.0f), i * glm::pi<float>() * 0.001f, { 1.0f, 0.0f, 0.0f })
						* glm::rotate(glm::mat4(1.0f), i * glm::pi<float>() * 0.03f, { 0.0f, 1.0f, 0.0f })
						* glm::rotate(glm::mat4(1.0f), i * glm::pi<float>() * (transformBase - physics.simulationDuration * transformSpeed), { 0.0f, 0.0f, 1.0f })
						* glm::translate(glm::mat4(1.0f), { i * 0.0005f, i * 0.0007f, i * 0.0009f });
					});
				crosses.state = ComponentState::Changed;
#endif
			}

			{
				auto& camera2D = Globals::Components().camera2D();
				auto& camera3D = Globals::Components().camera3D();
				camera3D.position = { camera2D.details.position * 0.2f, camera2D.details.projectionHSize * 0.4f + 70.0f };
				camera3D.fov = 30.0f / 360.0f * glm::two_pi<float>();
			}
		}

	private:
		std::array<CM::Texture, 4> planeTextures;
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
		ComponentId marbleTexture = 0;

		std::array<CM::AnimatedTexture, 4> flameAnimatedTextureForPlayers;
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
		ComponentId crossesId = 0;

		Tools::PlayersHandler playersHandler;
		Tools::MissilesHandler missilesHandler;

		std::unordered_set<const b2Body*> lowResBodies;

		float transformBase = 0.0304f;
		std::future<void> transformFuture;
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
		impl->gameStep();
		impl->decorationStep();
	}
}
