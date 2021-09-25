#include "playground.hpp"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include <globals.hpp>

#include <systems/deferredActions.hpp>

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
#include <components/missile.hpp>
#include <components/collisionHandler.hpp>
#include <components/shockwave.hpp>
#include <components/functor.hpp>

#include <ogl/uniformControllers.hpp>
#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/julia.hpp>
#include <ogl/shaders/texturedColorThreshold.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/shaders/particles.hpp>
#include <ogl/renderingHelpers.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>

#include <collisionBits.hpp>

namespace Levels
{
	class Playground::Impl
	{
	public:
		void setPhysicsSettings() const
		{
			Globals::Components().physics() = Components::Physics({ 0.0f, 0.0f });
		}

		void setGraphicsSettings() const
		{
			Globals::Components().graphicsSettings().defaultColor = { 0.7f, 0.7f, 0.7f, 1.0f };
		}

		void loadTextures()
		{
			auto& texturesDef = Globals::Components().texturesDef();

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

			missile1Texture = texturesDef.size();
			texturesDef.emplace_back("textures/missile1.png");
			texturesDef.back().minFilter = GL_LINEAR;
			texturesDef.back().translate = glm::vec2(-0.6f, -0.5f);
			texturesDef.back().scale = glm::vec2(0.3f, 0.4f);

			missile2Texture = texturesDef.size();
			texturesDef.emplace_back("textures/missile2.png");
			texturesDef.back().minFilter = GL_LINEAR;
			texturesDef.back().translate = glm::vec2(-0.5f, -0.5f);
			texturesDef.back().scale = glm::vec2(0.4f, 0.45f);

			explosionTexture = texturesDef.size();
			texturesDef.emplace_back("textures/explosion.png");
		}

		void createBackground()
		{
			Tools::CreateJuliaBackground(juliaShaders, []() { return Globals::Components().players()[0].getCenter() * 0.0001f; });
		}

		void createForeground() const
		{
			Tools::CreateFogForeground(2, 0.02f, fogTexture, [&, fogAlphaFactor = 1.0f, fogTargetAlphaFactor = 1.0f]() mutable {
				if (explosionFrame)
				{
					const float maxAlphaFactor = 1.5f;
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
				fogAlphaFactor += (fogTargetAlphaFactor - fogAlphaFactor) * Globals::Components().physics().frameDuration;
				return glm::vec4(1.0f, 1.0f, 1.0f, fogAlphaFactor);
				});
		}

		void createPlayers()
		{
			player1Handler = Tools::CreatePlayerPlane(rocketPlaneTexture, flameAnimation1Texture);
		}

		void launchMissile()
		{
			const float launchDistanceFromCenter = 1.2f;
			const auto relativeLaunchPos = glm::vec2(glm::cos(Globals::Components().players()[0].getAngle() + glm::half_pi<float>()),
				glm::sin(Globals::Components().players()[0].getAngle() + glm::half_pi<float>())) * (missileFromLeft ? launchDistanceFromCenter : -launchDistanceFromCenter);

			auto missileHandler = Tools::CreateMissile(Globals::Components().players()[0].getCenter() + relativeLaunchPos,
				Globals::Components().players()[0].getAngle(), 5.0f, Globals::Components().players()[0].getVelocity(), missile2Texture, flameAnimation1Texture);
			missilesToHandlers.emplace(missileHandler.missileId, std::move(missileHandler));

			missileFromLeft = !missileFromLeft;
		}

		void createDynamicWalls()
		{
			auto& wall1 = *Globals::Components().dynamicWalls().emplace_back(
				Tools::CreateBoxBody({ 5.0f, -5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f), woodTexture).body;
			auto& wall2 = *Globals::Components().dynamicWalls().emplace_back(
				Tools::CreateBoxBody({ 5.0f, 5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f), woodTexture).body;
			wall1.GetFixtureList()->SetRestitution(0.5f);
			wall2.GetFixtureList()->SetRestitution(0.5f);
			Tools::PinBodies(wall1, wall2, { 5.0f, 0.0f });
			Globals::Components().dynamicWalls().back().renderingSetup = Tools::MakeUniqueRenderingSetup([
				textureTranslateUniform = Uniforms::UniformController2f()
			](Shaders::ProgramId program) mutable {
				if (!textureTranslateUniform.isValid()) textureTranslateUniform = Uniforms::UniformController2f(program, "textureTranslate");
				const float simulationDuration = Globals::Components().physics().simulationDuration;
				textureTranslateUniform.setValue({ glm::cos(simulationDuration * 0.1f), glm::sin(simulationDuration * 0.1f) });
				return nullptr;
			});

			for (const float pos : {-30.0f, 30.0f})
			{
				Globals::Components().dynamicWalls().emplace_back(Tools::CreateCircleBody({ 0.0f, pos }, 5.0f, b2_dynamicBody, 0.01f), woodTexture,
					Tools::MakeUniqueRenderingSetup([this](auto)
					{
						Tools::MVPInitialization(texturedColorThresholdShaders);
						Tools::StaticTexturedRenderInitialization(texturedColorThresholdShaders, woodTexture, true);
						const float simulationDuration = Globals::Components().physics().simulationDuration;
						texturedColorThresholdShaders.invisibleColorUniform.setValue({ 1.0f, 1.0f, 1.0f });
						texturedColorThresholdShaders.invisibleColorThresholdUniform.setValue((-glm::cos(simulationDuration * 0.5f) + 1.0f) * 0.5f);
						return nullptr;
					}),
					texturedColorThresholdShaders.getProgramId());
				Globals::Components().dynamicWalls().emplace_back(Tools::CreateCircleBody({ pos, 0.0f }, 10.0f, b2_dynamicBody, 0.01f));
				Globals::Components().dynamicWalls().back().renderingSetup = Tools::MakeUniqueRenderingSetup([
					basicProgram = Shaders::Programs::BasicAccessor()
				](Shaders::ProgramId program) mutable {
					if (!basicProgram.isValid()) basicProgram = program;
					basicProgram.colorUniform.setValue({ 1.0f, 1.0f, 1.0f, 0.0f });
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
				}), roseTexture);
				Globals::Components().midgroundDecorations().back().texCoord = Tools::CreateTexCoordOfRectangle();
				Globals::Components().midgroundDecorations().back().renderingSetup = Tools::MakeUniqueRenderingSetup([
					texturedProgram = Shaders::Programs::TexturedAccessor(),
					wallId = Globals::Components().dynamicWalls().size() - 1
				](Shaders::ProgramId program) mutable {
					if (!texturedProgram.isValid()) texturedProgram = program;
					texturedProgram.colorUniform.setValue({ 1.0f, 1.0f, 1.0f,
						(glm::sin(Globals::Components().physics().simulationDuration * glm::two_pi<float>()) + 1.0f) / 2.0f + 0.5f });
					texturedProgram.modelUniform.setValue(Globals::Components().dynamicWalls()[wallId].getModelMatrix());
					return nullptr;
				});
			}
			Globals::Components().midgroundDecorations().back().resolutionMode = ResolutionMode::PixelArtBlend0;
			lowResBodies.insert(Globals::Components().dynamicWalls().back().body.get());
		}

		void createStaticWalls() const
		{
			const float levelHSize = 50.0f;
			const float bordersHGauge = 50.0f;

			Globals::Components().staticWalls().emplace_back(Tools::CreateBoxBody({ -levelHSize - bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize + bordersHGauge * 2 }), spaceRockTexture);
			Globals::Components().nearMidgroundDecorations().emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.5f, 1.5f }, { { -levelHSize, -levelHSize }, { levelHSize, -levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), weedTexture);
			Globals::Components().nearMidgroundDecorations().back().texCoord = Tools::CreateTexCoordOfRectangle();

			Globals::Components().staticWalls().emplace_back(Tools::CreateBoxBody({ levelHSize + bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize + bordersHGauge * 2 }), spaceRockTexture);
			Globals::Components().nearMidgroundDecorations().emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.5f, 1.5f }, { { -levelHSize, levelHSize }, { levelHSize, levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), weedTexture);
			Globals::Components().nearMidgroundDecorations().back().texCoord = Tools::CreateTexCoordOfRectangle();

			Globals::Components().staticWalls().emplace_back(Tools::CreateBoxBody({ 0.0f, -levelHSize - bordersHGauge },
				{ levelHSize + bordersHGauge * 2, bordersHGauge }), spaceRockTexture);
			Globals::Components().nearMidgroundDecorations().emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.5f, 1.5f }, { { -levelHSize, -levelHSize }, { -levelHSize, levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), weedTexture);
			Globals::Components().nearMidgroundDecorations().back().texCoord = Tools::CreateTexCoordOfRectangle();

			Globals::Components().staticWalls().emplace_back(Tools::CreateBoxBody({ 0.0f, levelHSize + bordersHGauge },
				{ levelHSize + bordersHGauge * 2, bordersHGauge }), spaceRockTexture);
			Globals::Components().nearMidgroundDecorations().emplace_back(Tools::CreatePositionsOfLineOfRectangles({ 1.5f, 1.5f }, { { levelHSize, -levelHSize }, { levelHSize, levelHSize } },
				{ 2.0f, 3.0f }, { 0.0f, glm::two_pi<float>() }, { 0.7f, 1.3f }), weedTexture);
			Globals::Components().nearMidgroundDecorations().back().texCoord = Tools::CreateTexCoordOfRectangle();
		}

		void createGrapples() const
		{
			Globals::Components().grapples().emplace_back(Tools::CreateCircleBody({ 0.0f, 10.0f }, 1.0f), 15.0f, orbTexture);
			Globals::Components().grapples().emplace_back(Tools::CreateCircleBody({ 0.0f, -10.0f }, 1.0f), 15.0f, orbTexture);
			Globals::Components().grapples().back().renderingSetup = Tools::MakeUniqueRenderingSetup([
				colorUniform = Uniforms::UniformController4f()
			](Shaders::ProgramId program) mutable {
				if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
				colorUniform.setValue({ 1.0f, 1.0f, 1.0f,
					(glm::sin(Globals::Components().physics().simulationDuration / 3.0f * glm::two_pi<float>()) + 1.0f) / 2.0f });
				return nullptr;
			});
			Globals::Components().grapples().emplace_back(Tools::CreateCircleBody({ -10.0f, -30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f), 30.0f,
				orbTexture);
			auto& grapple = Globals::Components().grapples().emplace_back(Tools::CreateCircleBody({ -10.0f, 30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f), 30.0f);
			grapple.renderingSetup = Tools::MakeUniqueRenderingSetup([
				colorUniform = Uniforms::UniformController4f()
			](Shaders::ProgramId program) mutable {
				if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
				colorUniform.setValue({0.0f, 0.0f, 0.0f, 0.0f});
				return nullptr;
			});

			Globals::Components().farMidgroundDecorations().emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 2.2f, 2.2f }), roseTexture);
			Globals::Components().farMidgroundDecorations().back().texCoord = Tools::CreateTexCoordOfRectangle();
			Globals::Components().farMidgroundDecorations().back().renderingSetup = Tools::MakeUniqueRenderingSetup([&,
				modelUniform = Uniforms::UniformControllerMat4f()
			](Shaders::ProgramId program) mutable {
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
				modelUniform.setValue(grapple.getModelMatrix());
				return nullptr;
			});
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
			EmplaceIdComponent(Globals::Components().beginCollisionHandlers(), { CollisionBits::missileBit, CollisionBits::all, [this](const auto& fixtureA, const auto& fixtureB) {
				for (const auto* fixture : { &fixtureA, &fixtureB })
				if (fixture->GetFilterData().categoryBits == CollisionBits::missileBit)
				{
					const auto& otherFixture = fixture == &fixtureA ? fixtureB : fixtureA;
					const auto& body = *fixture->GetBody();
					missilesToHandlers.erase(Tools::AccessUserData(body).componentId);
					Tools::CreateExplosion(particlesShaders, ToVec2<glm::vec2>(body.GetWorldCenter()), explosionTexture, 1.0f, 64, 4,
						lowResBodies.count(otherFixture.GetBody()) ? ResolutionMode::LowPixelArtBlend1 : ResolutionMode::LowestLinearBlend1);

					explosionFrame = true;
				}
			} });
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
		}

	private:
		Shaders::Programs::Julia juliaShaders;
		Shaders::Programs::TexturedColorThreshold texturedColorThresholdShaders;
		Shaders::Programs::Particles particlesShaders;

		unsigned rocketPlaneTexture = 0;
		unsigned spaceRockTexture = 0;
		unsigned woodTexture = 0;
		unsigned orbTexture = 0;
		unsigned weedTexture = 0;
		unsigned roseTexture = 0;
		unsigned fogTexture = 0;
		unsigned flameAnimation1Texture = 0;
		unsigned missile1Texture = 0;
		unsigned missile2Texture = 0;
		unsigned explosionTexture = 0;

		Tools::PlayerPlaneHandler player1Handler;

		float durationToLaunchMissile = 0.0f;
		bool missileFromLeft = false;

		int prevWheel = 0;
		float projectionHSizeBase = 20.0f;

		bool explosionFrame = false;

		std::unordered_map<ComponentId, Tools::MissileHandler> missilesToHandlers;
		std::unordered_set<const b2Body*> lowResBodies;
	};

	Playground::Playground():
		impl(std::make_unique<Impl>())
	{
		impl->setPhysicsSettings();
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->createBackground();
		impl->createPlayers();
		impl->createDynamicWalls();
		impl->createStaticWalls();
		impl->createGrapples();
		impl->createForeground();
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
