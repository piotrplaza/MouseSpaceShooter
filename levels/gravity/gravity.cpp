#include "gravity.hpp"

#include <components/screenInfo.hpp>
#include <components/physics.hpp>
#include <components/renderingSetup.hpp>
#include <components/plane.hpp>
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

#include <ogl/uniforms.hpp>
#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/julia.hpp>
#include <ogl/shaders/texturedColorThreshold.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/shaders/particles.hpp>
#include <ogl/renderingHelpers.hpp>

#include <globals/shaders.hpp>
#include <globals/components.hpp>
#include <globals/collisionBits.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>

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
			textures.back().translate = glm::vec2(0.4f, 0.0f);
			textures.back().scale = glm::vec2(1.6f, 1.8f);

			spaceRockTexture = textures.size();
			textures.emplace_back("textures/space rock.jpg", GL_MIRRORED_REPEAT);
			textures.back().scale = glm::vec2(5.0f);

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
		}

		void setAnimations()
		{
			flame1AnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().push_back(Components::AnimatedTexture(
				flame1AnimationTexture, { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
				AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal));
			Globals::Components().animatedTextures().back().start(true);

			/*flame2AnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().push_back(Components::AnimatedTexture(
				flame1AnimationTexture, { 500, 498 }, { 2, 0 }, { 61, 120 }, { 8, 4 }, { 62.5f, 124.9f }, 0.02f, 0,
				AnimationLayout::Horizontal, AnimationPlayback::Backward, AnimationPolicy::Repeat,
				{ 4.1f, 2.0f }, { 1.2f, -1.2f }));
			Globals::Components().animatedTextures().back().start();*/
		}

		void createBackground()
		{
			Tools::CreateJuliaBackground(juliaShaders, [this]() {
				return Globals::Components().planes()[player1Handler.planeId].getCenter() * 0.0001f; });
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
			player1Handler = Tools::CreatePlane(rocketPlaneTexture, flame1AnimatedTexture, {0.0f, -50.0f}, glm::half_pi<float>());
		}

		void launchMissile()
		{
			auto missileHandler = Tools::CreateMissile(Globals::Components().planes()[player1Handler.planeId].getCenter(),
				Globals::Components().planes()[player1Handler.planeId].getAngle(), 5.0f, Globals::Components().planes()[player1Handler.planeId].getVelocity(),
				missile2Texture, flame1AnimatedTexture);
			missilesToHandlers.emplace(missileHandler.missileId, std::move(missileHandler));
		}

		void createMovableWalls()
		{
			debrisBegin = Globals::Components().structures().size();

			for (int i = 0; i < 500; ++i)
			{
				const float angle = Tools::Random(0.0f, glm::two_pi<float>());
				const glm::vec2 pos = glm::vec2(glm::cos(angle), glm::sin(angle)) * 20.0f;
				Globals::Components().structures().emplace_back(
					Tools::CreateBoxBody(pos, { Tools::Random(0.1f, 1.0f), Tools::Random(0.1f, 1.0f) }, angle, b2_dynamicBody, 0.02f),
					TCM::Texture(spaceRockTexture));
			}

			debrisEnd = Globals::Components().structures().size();
		}

		void createStationaryWalls() const
		{
		}

		void createGrapples()
		{
			auto& grapple = EmplaceDynamicComponent(Globals::Components().grapples(), { Tools::CreateCircleBody({ 0.0f, 0.0f }, 20.0f), TCM::Texture(orbTexture) });
			grapple.influenceRadius = 60.0f;
			planetId = grapple.getComponentId();
		}

		void setCamera() const
		{
			const auto& plane = Globals::Components().planes()[player1Handler.planeId];
			const auto& planet = Globals::Components().grapples()[planetId];

			Globals::Components().camera().targetProjectionHSizeF = [&]() {
				Globals::Components().camera().projectionTransitionFactor = Globals::Components().physics().frameDuration * 6;
				return glm::distance(plane.getCenter(), planet.getCenter()) * 0.6f + glm::length(plane.getVelocity()) * 0.2f;
			};
			Globals::Components().camera().targetPositionF = [&]() {
				Globals::Components().camera().positionTransitionFactor = Globals::Components().physics().frameDuration * 6;
				return (plane.getCenter() + planet.getCenter()) / 2.0f + glm::vec2(glm::cos(plane.getAngle()), glm::sin(plane.getAngle())) * 5.0f + plane.getVelocity() * 0.4f;
			};
		}

		void setCollisionCallbacks()
		{
			EmplaceDynamicComponent(Globals::Components().beginCollisionHandlers(), { Globals::CollisionBits::missileBit, Globals::CollisionBits::all,
				[this](const auto& fixtureA, const auto& fixtureB) {
					for (const auto* fixture : { &fixtureA, &fixtureB })
					if (fixture->GetFilterData().categoryBits == Globals::CollisionBits::missileBit)
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
			EmplaceDynamicComponent(Globals::Components().frameSetups(), { [&]()
				{
					explosionFrame = false;
				} });
		}

		void step()
		{
			const auto& mouseState = Globals::Components().mouseState();
			const glm::vec2 mouseDelta = { mouseState.getMouseDelta().x, -mouseState.getMouseDelta().y };
			auto& player1Controls = Globals::Components().planes()[player1Handler.planeId].controls;

			player1Controls.turningDelta = mouseDelta;
			player1Controls.autoRotation = mouseState.rmb;
			player1Controls.throttling = mouseState.rmb;
			player1Controls.magneticHook = mouseState.xmb1;

			if (mouseState.lmb)
			{
				if (durationToLaunchMissile <= 0.0f)
				{
					launchMissile();
					durationToLaunchMissile = 0.05f;
				}
				else durationToLaunchMissile -= Globals::Components().physics().frameDuration;
			}
			else durationToLaunchMissile = 0.0f;

			const float gravityFactor = 0.02f;

			for (size_t i = debrisBegin; i != debrisEnd; ++i)
			{
				auto& debris = Globals::Components().structures()[i];
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

		unsigned flame1AnimatedTexture = 0;

		Tools::PlaneHandler player1Handler;

		float durationToLaunchMissile = 0.0f;

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
		impl->createMovableWalls();
		impl->createStationaryWalls();
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
