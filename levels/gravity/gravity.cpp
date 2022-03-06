#include "gravity.hpp"

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
	class Gravity::Impl
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
			textures.back().translate = glm::vec2(0.15f, 0.0f);
			textures.back().scale = glm::vec2(1.7f);

			spaceRockTexture = textures.size();
			textures.emplace_back("textures/space rock.jpg", GL_MIRRORED_REPEAT);
			textures.back().scale = glm::vec2(20.0f);

			woodTexture = textures.size();
			textures.emplace_back("textures/wood.jpg", GL_MIRRORED_REPEAT);
			textures.back().scale = glm::vec2(16.0f);

			orbTexture = textures.size();
			textures.emplace_back("textures/orb.png");
			textures.back().scale = glm::vec2(40.0f);

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
			textures.back().scale = glm::vec2(50.0f, 50.0f);

			ppTexture = textures.size();
			textures.emplace_back("textures/pp.png");

			skullTexture = textures.size();
			textures.emplace_back("textures/skull_rot.png");
			textures.back().translate = glm::vec2(0.04f, 0.47f);
			textures.back().scale = glm::vec2(0.48f, 0.44f);

			avatarTexture = textures.size();
			textures.emplace_back("textures/avatar_rot.png");
			textures.back().translate = glm::vec2(0.08f, 0.495f);
			textures.back().scale = glm::vec2(0.24f, 0.32f);
			textures.back().darkToTransparent = true;
		}

		void setAnimations()
		{
			flame1AnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().push_back(Components::AnimatedTexture(
				flame1AnimationTexture, { 500, 498 }, { 2, 0 }, { 61, 120 }, { 8, 4 }, { 62.5f, 124.9f }, 0.02f, 0,
				AnimationLayout::Horizontal, AnimationPlayback::Backward, AnimationPolicy::Repeat,
				{ 4.1f, 1.65f }, { 1.0f, 1.0f }));
			Globals::Components().animatedTextures().back().start();

			flame2AnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().push_back(Components::AnimatedTexture(
				flame1AnimationTexture, { 500, 498 }, { 2, 0 }, { 61, 120 }, { 8, 4 }, { 62.5f, 124.9f }, 0.02f, 0,
				AnimationLayout::Horizontal, AnimationPlayback::Backward, AnimationPolicy::Repeat,
				{ 4.1f, 2.0f }, { 1.2f, -1.2f }));
			Globals::Components().animatedTextures().back().start();
		}

		void createBackground()
		{
			Tools::CreateJuliaBackground(juliaShaders, []() { return Globals::Components().players()[1].getCenter() * 0.0001f; });
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
		}

		void createPlayers()
		{
			player1Handler = Tools::CreatePlayerPlane(rocketPlaneTexture, flame1AnimatedTexture, {0.0f, -50.0f}, glm::half_pi<float>());
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
			debrisBegin = Globals::Components().dynamicWalls().size();

			for (int i = 0; i < 500; ++i)
			{
				const float angle = Tools::Random(0.0f, glm::two_pi<float>());
				const glm::vec2 pos = glm::vec2(glm::cos(angle), glm::sin(angle)) * 20.0f;
				Globals::Components().dynamicWalls().emplace_back(
					Tools::CreateBoxBody(pos, { Tools::Random(0.1f, 1.0f), Tools::Random(0.1f, 1.0f) }, angle, b2_dynamicBody, 0.02f),
					TCM::Texture(spaceRockTexture));
			}

			debrisEnd = Globals::Components().dynamicWalls().size();
		}

		void createStaticWalls() const
		{
		}

		void createGrapples()
		{
			planetId = Globals::Components().grapples().size();
			Globals::Components().grapples().emplace_back(Tools::CreateCircleBody({ 0.0f, 0.0f }, 20.0f), 60.0f, TCM::Texture(orbTexture));
		}

		void setCamera() const
		{
			const auto& player = Globals::Components().players()[player1Handler.playerId];
			const auto& planet = Globals::Components().grapples()[planetId];

			Globals::Components().camera().targetProjectionHSizeF = [&]() {
				Globals::Components().camera().projectionTransitionFactor = Globals::Components().physics().frameDuration * 6;
				return glm::distance(player.getCenter(), planet.getCenter()) * 0.6f + glm::length(player.getVelocity()) * 0.2f;
			};
			Globals::Components().camera().targetPositionF = [&]() {
				Globals::Components().camera().positionTransitionFactor = Globals::Components().physics().frameDuration * 6;
				return (player.getCenter() + planet.getCenter()) / 2.0f + glm::vec2(glm::cos(player.getAngle()), glm::sin(player.getAngle())) * 5.0f + player.getVelocity() * 0.4f;
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
						Tools::CreateExplosion(particlesShaders, ToVec2<glm::vec2>(body.GetWorldCenter()), explosionTexture, 1.0f, 64, 4,
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
					durationToLaunchMissile = 0.05f;
				}
				else durationToLaunchMissile -= Globals::Components().physics().frameDuration;
			}
			else durationToLaunchMissile = 0.0f;

			projectionHSizeBase = std::clamp(projectionHSizeBase + (prevWheel - Globals::Components().mouseState().wheel) * 5.0f, 5.0f, 100.0f);
			prevWheel = Globals::Components().mouseState().wheel;

			const float gravityFactor = 0.02f;

			for (size_t i = debrisBegin; i != debrisEnd; ++i)
			{
				auto& debris = Globals::Components().dynamicWalls()[i];
				const auto& planet = Globals::Components().grapples()[planetId];
				const auto gravityDiff = (planet.getCenter() - debris.getCenter()) * gravityFactor;
				const auto gravityVecNorm = glm::normalize(gravityDiff);
				const auto gravityVecDist = glm::length(gravityDiff);
				const auto gravityVec = gravityVecNorm * glm::pow(gravityVecDist, 2.0f);

				debris.body->ApplyForce({ gravityVec.x, gravityVec.y }, { 0.0f, 0.0f }, true);
			}
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

		size_t debrisBegin = 0;
		size_t debrisEnd = 0;
		size_t planetId = 0;

		std::unordered_map<ComponentId, Tools::MissileHandler> missilesToHandlers;
		std::unordered_set<const b2Body*> lowResBodies;
	};

	Gravity::Gravity():
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

	Gravity::~Gravity() = default;

	void Gravity::step()
	{
		impl->step();
	}
}
