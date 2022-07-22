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
#include <ogl/shaders/texturedColorThreshold.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/renderingHelpers.hpp>

#include <globals/shaders.hpp>
#include <globals/components.hpp>
#include <globals/collisionBits.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>
#include <tools/b2Helpers.hpp>

#include <glm/gtx/vector_angle.hpp>

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
			Tools::CreateJuliaBackground([this]() {
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
			const float initVelocity = 5.0f;
			auto& plane = Globals::Components().planes()[player1Handler.planeId];

			auto missileHandler = Tools::CreateMissile(plane.getCenter(), plane.getAngle(), 5.0f, plane.getVelocity(),
				glm::vec2(glm::cos(plane.getAngle()), glm::sin(plane.getAngle())) * initVelocity, missile2Texture, flame1AnimatedTexture);

			missilesToHandlers.emplace(missileHandler.missileId, std::move(missileHandler));
		}

		void createMovableWalls()
		{
			debrisBegin = Globals::Components().walls().size();

			for (int i = 0; i < 500; ++i)
			{
				const float angle = Tools::Random(0.0f, glm::two_pi<float>());
				const glm::vec2 pos = glm::vec2(glm::cos(angle), glm::sin(angle)) * 20.0f;
				Globals::Components().walls().emplace_back(
					Tools::CreateBoxBody(pos, { Tools::Random(0.1f, 1.0f), Tools::Random(0.1f, 1.0f) }, angle, b2_dynamicBody, 0.02f),
					TCM::Texture(spaceRockTexture));
			}

			debrisEnd = Globals::Components().walls().size();
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
				return (glm::distance(plane.getCenter(), planet.getCenter()) * 0.6f + glm::length(plane.getVelocity()) * 0.2f) * projectionHSizeBase * 0.2f;
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
						const auto& targetFixture = fixture == &fixtureA ? fixtureB : fixtureA;
						const auto& missileBody = *fixture->GetBody();
						missilesToHandlers.erase(std::get<TCM::Missile>(Tools::AccessUserData(missileBody).bodyComponentVariant).id);
						Tools::CreateExplosion(Tools::ExplosionParams().center(ToVec2<glm::vec2>(missileBody.GetWorldCenter())).explosionTexture(explosionTexture).resolutionMode(
							lowResBodies.count(targetFixture.GetBody()) ? ResolutionMode::LowPixelArtBlend1 : ResolutionMode::LowestLinearBlend1));

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
			auto& player1Controls = Globals::Components().planes()[player1Handler.planeId].controls;

			player1Controls.turningDelta = mouseState.getWorldSpaceDelta();
			player1Controls.autoRotation = mouseState.pressing.rmb;
			player1Controls.throttling = mouseState.pressing.rmb;
			player1Controls.magneticHook = mouseState.pressing.xmb1;

			if (mouseState.pressing.lmb)
			{
				if (durationToLaunchMissile <= 0.0f)
				{
					launchMissile();
					durationToLaunchMissile = 0.1f;
				}
				else durationToLaunchMissile -= Globals::Components().physics().frameDuration;
			}
			else durationToLaunchMissile = 0.0f;

			auto applyGravity = [&](auto& component, float mM)
			{
				const auto& planet = Globals::Components().grapples()[planetId];
				const auto gravityDiff = planet.getCenter() - component.getCenter();
				const auto gravityVecDist = glm::length(gravityDiff);
				const auto gravityVecNorm = glm::normalize(gravityDiff);
				const auto gravityVec = mM / glm::pow(gravityVecDist, 2.0f) * gravityVecNorm;

				component.body->ApplyForce({ gravityVec.x, gravityVec.y }, { 0.0f, 0.0f }, true);
			};

			for (size_t i = debrisBegin; i != debrisEnd; ++i)
				applyGravity(Globals::Components().walls()[i], 400.0f);

			for (auto& [id, missile] : Globals::Components().missiles())
			{
				applyGravity(missile, 4000.0f);
				missile.body->SetTransform(missile.body->GetPosition(), glm::orientedAngle({ 1.0f, 0.0f },
					glm::normalize(ToVec2<glm::vec2>(missile.body->GetLinearVelocity()) - missilesToHandlers[id].referenceVelocity)));
			}

			projectionHSizeBase = std::clamp(projectionHSizeBase + mouseState.pressed.wheel * -5.0f, 5.0f, 100.0f);
		}

	private:
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

		float projectionHSizeBase = 5.0f;

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
