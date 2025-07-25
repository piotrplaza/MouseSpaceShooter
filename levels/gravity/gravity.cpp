#include "gravity.hpp"

#include <components/systemInfo.hpp>
#include <components/physics.hpp>
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
#include <components/collisionHandler.hpp>
#include <components/shockwave.hpp>
#include <components/functor.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/blendingTexture.hpp>
#include <components/animatedTexture.hpp>

#include <ogl/uniformsUtils.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/renderingHelpers.hpp>

#include <globals/shaders.hpp>
#include <globals/components.hpp>
#include <globals/collisionBits.hpp>

#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>
#include <tools/particleSystemHelpers.hpp>
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
			Globals::Components().graphicsSettings().defaultColorF = glm::vec4{ 0.7f, 0.7f, 0.7f, 1.0f };
			Globals::Components().mainFramebufferRenderer().renderer = Tools::Demo3DRotatedFullscreenRenderer(Globals::Shaders().textured());
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().staticTextures();

			plane1Texture = textures.size();
			textures.emplace("textures/plane 1.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.6f, 1.8f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			spaceRockTexture = textures.size();
			textures.emplace("textures/space rock.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(5.0f);

			woodTexture = textures.size();
			textures.emplace("textures/wood.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(16.0f);

			orbTexture = textures.size();
			textures.emplace("textures/orb.png");
			textures.last().scale = glm::vec2(40.0f);

			weedTexture = textures.size();
			textures.emplace("textures/weed.png");
			textures.last().minFilter = GL_LINEAR_MIPMAP_LINEAR;

			roseTexture = textures.size();
			textures.emplace("textures/rose.png");
			textures.last().minFilter = GL_LINEAR_MIPMAP_LINEAR;

			fogTexture = textures.size();
			textures.emplace("textures/fog.png");

			flame1AnimationTexture = textures.size();
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
		}

		void setAnimations()
		{
			flame1AnimatedTexture = Globals::Components().staticAnimatedTextures().size();
			Globals::Components().staticAnimatedTextures().add({ CM::Texture(flame1AnimationTexture, true), { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
				AnimationData::Direction::Backward, AnimationData::Mode::Repeat, AnimationData::TextureLayout::Horizontal });
			Globals::Components().staticAnimatedTextures().last().start(true);

			/*flame2AnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().push_back(Components::AnimatedTexture(
				flame1AnimationTexture, { 500, 498 }, { 2, 0 }, { 61, 120 }, { 8, 4 }, { 62.5f, 124.9f }, 0.02f, 0,
				AnimationLayout::Horizontal, AnimationPlayback::Backward, AnimationData::Mode::Repeat,
				{ 4.1f, 2.0f }, { 1.2f, -1.2f }));
			Globals::Components().animatedTextures().back().start();*/
		}

		void createBackground()
		{
			Tools::CreateJuliaBackground(Tools::JuliaParams{}.juliaCOffsetF([this]() {
				return Globals::Components().planes()[player1Id].originF() * 0.0001f; }));
		}

		void createForeground()
		{
			//Tools::CreateFogForeground(2, 0.02f, CM::Texture(fogTexture, true), [&, fogTargetAlphaFactor = 1.0f]() mutable {
			//	if (explosionFrame)
			//	{
			//		const float maxAlphaFactor = 2.0f;
			//		fogTargetAlphaFactor = fogTargetAlphaFactor < maxAlphaFactor
			//			? fogTargetAlphaFactor + Globals::Components().physics().frameDuration * 10.0f
			//			: maxAlphaFactor;
			//	}
			//	else
			//	{
			//		fogTargetAlphaFactor = fogTargetAlphaFactor > 1.0f
			//			? fogTargetAlphaFactor - Globals::Components().physics().frameDuration * 0.2f
			//			: 1.0f;
			//	}
			//	fogAlphaFactor += (fogTargetAlphaFactor - fogAlphaFactor) * 0.05f * Globals::Components().physics().frameDuration;
			//	return glm::vec4(1.0f, 1.0f, 1.0f, fogAlphaFactor);
			//});
		}

		void createAdditionalDecorations() const
		{
		}

		void createPlayers()
		{
			player1Id = Tools::CreatePlane(Tools::CreateTrianglesBody({ { glm::vec2{2.0f, 0.0f}, glm::vec2{-1.0f, 1.0f}, glm::vec2{-1.0f, -1.0f} } }, Tools::GetDefaultParamsForPlaneBody()),
				CM::Texture(plane1Texture, true), CM::AnimatedTexture(flame1AnimatedTexture, true), Tools::PlaneParams().position({ 0.0f, -50.0f }).angle(glm::half_pi<float>()));
		}

		void launchMissile()
		{
			const float initExplosionVelocity = 5.0f;
			auto& plane = Globals::Components().planes()[player1Id];

			auto missileHandler = Tools::CreateMissile(plane.originF(), plane.getAngle(), 5.0f, plane.getVelocity(),
				glm::vec2(glm::cos(plane.getAngle()), glm::sin(plane.getAngle())) * initExplosionVelocity, CM::Texture(missile2Texture, true), CM::AnimatedTexture(flame1AnimatedTexture, true));

			missilesToHandlers.emplace(missileHandler.missile.componentId, std::move(missileHandler));
		}

		void createMovableWalls()
		{
			debrisBegin = Globals::Components().staticWalls().size();

			for (int i = 0; i < 500; ++i)
			{
				const float angle = Tools::RandomFloat(0.0f, glm::two_pi<float>());
				const glm::vec2 pos = glm::vec2(glm::cos(angle), glm::sin(angle)) * 20.0f;
				Globals::Components().staticWalls().emplace(
					Tools::CreateBoxBody({ Tools::RandomFloat(0.1f, 1.0f), Tools::RandomFloat(0.1f, 1.0f) },
						Tools::BodyParams().position(pos).angle(angle).bodyType(b2_dynamicBody).density(0.02f)),
					CM::Texture(spaceRockTexture, true));
			}

			debrisEnd = Globals::Components().staticWalls().size();
		}

		void createStationaryWalls() const
		{
		}

		void createGrapples()
		{
			auto& grapple = Globals::Components().grapples().emplace(Tools::CreateDiscBody(20.0f,
				Tools::BodyParams()), CM::Texture(orbTexture, true));
			grapple.range = 100.0f;
			planetId = grapple.getComponentId();
		}

		void setCamera() const
		{
			const auto& plane = Globals::Components().planes()[player1Id];
			const auto& planet = Globals::Components().grapples()[planetId];

			Globals::Components().camera2D().projectionTransitionFactor = 6;
			Globals::Components().camera2D().positionTransitionFactor = 6;
			Globals::Components().camera2D().targetPositionAndProjectionHSizeF = [&]() {
				return glm::vec3((plane.getOrigin2D() + planet.getOrigin2D()) / 2.0f + glm::vec2(glm::cos(plane.getAngle()), glm::sin(plane.getAngle())) * 5.0f + plane.getVelocity() * 0.4f,
					(glm::distance(plane.getOrigin2D(), planet.getOrigin2D()) * 0.6f + glm::length(plane.getVelocity()) * 0.2f) * projectionHSizeBase * 0.2f);
			};
		}

		void setCollisionCallbacks()
		{
			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::projectile, Globals::CollisionBits::all,
				[this](const auto& fixtureA, const auto& fixtureB) {
					for (const auto* fixture : { &fixtureA, &fixtureB })
					if (fixture->GetFilterData().categoryBits == Globals::CollisionBits::projectile)
					{
						const auto& targetFixture = fixture == &fixtureA ? fixtureB : fixtureA;
						const auto& missileBody = *fixture->GetBody();
						missilesToHandlers.erase(std::get<CM::Missile>(Tools::AccessUserData(missileBody).bodyComponentVariant).componentId);
						Tools::CreateExplosion(Tools::ExplosionParams().center(ToVec2<glm::vec2>(missileBody.GetWorldCenter())).explosionTexture(CM::Texture(explosionTexture, true)));

						explosionFrame = true;
					}
				});
		}

		void setFramesRoutines()
		{
			Globals::Components().stepSetups().emplace([&]() { explosionFrame = false; return true; });
		}

		void step()
		{
			float mouseSensitivity = 0.01f;
			float gamepadSensitivity = 50.0f;

			const auto& physics = Globals::Components().physics();
			const auto& mouse = Globals::Components().mouse();
			const auto& gamepad = Globals::Components().gamepads()[0];
			auto& player1Controls = Globals::Components().planes()[player1Id].controls;

			player1Controls.turningDelta = mouse.getCartesianDelta() * mouseSensitivity +
				Tools::ApplyDeadzone(gamepad.lStick) * physics.frameDuration * gamepadSensitivity;
			player1Controls.autoRotation = (bool)std::max((float)mouse.pressing.rmb, gamepad.rTrigger);
			player1Controls.throttling = std::max((float)mouse.pressing.rmb, gamepad.rTrigger);
			player1Controls.grappleHook = mouse.pressing.xmb1 || gamepad.pressing.lShoulder || gamepad.lTrigger >= 0.5f;

			if (mouse.pressing.lmb || gamepad.pressing.x)
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
				const auto gravityDiff = planet.originF() - component.originF();
				const auto gravityVecDist = glm::length(gravityDiff);
				const auto gravityVecNorm = glm::normalize(gravityDiff);
				const auto gravityVec = mM / glm::pow(gravityVecDist, 2.0f) * gravityVecNorm;

				component.body->ApplyForce({ gravityVec.x, gravityVec.y }, { 0.0f, 0.0f }, true);
			};

			for (ComponentId i = debrisBegin; i != debrisEnd; ++i)
				applyGravity(Globals::Components().staticWalls()[i], 400.0f);

			for (auto& missile: Globals::Components().missiles())
			{
				if (missile.state == ComponentState::Outdated)
					continue;
				applyGravity(missile, 4000.0f);
				missile.body->SetTransform(missile.body->GetPosition(), glm::orientedAngle({ 1.0f, 0.0f },
					glm::normalize(ToVec2<glm::vec2>(missile.body->GetLinearVelocity()) - missilesToHandlers[missile.getComponentId()].referenceVelocity)));
			}

			if (mouse.pressing.mmb || gamepad.pressing.rShoulder)
				Globals::Components().physics().gameSpeed = std::clamp(Globals::Components().physics().gameSpeed +
					(mouse.pressed.wheel + gamepad.pressed.dUp * 1 + gamepad.pressed.dDown * -1) * 0.1f, 0.0f, 2.0f);
			else
				projectionHSizeBase = std::clamp(projectionHSizeBase + (mouse.pressed.wheel +
					gamepad.pressed.dUp * 1 + gamepad.pressed.dDown * -1) * -5.0f, 5.0f, 100.0f);
		}

	private:
		unsigned plane1Texture = 0;
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

		unsigned player1Id = 0;

		float durationToLaunchMissile = 0.0f;

		float projectionHSizeBase = 5.0f;

		bool explosionFrame = false;

		float fogAlphaFactor = 1.0f;

		ComponentId debrisBegin = 0;
		ComponentId debrisEnd = 0;
		ComponentId planetId = 0;

		std::unordered_map<CM::Missile, Tools::MissileHandler> missilesToHandlers;
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
