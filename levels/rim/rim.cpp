#include "rim.hpp"

#include <components/graphicsSettings.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/camera.hpp>
#include <components/plane.hpp>
#include <components/wall.hpp>
#include <components/collisionHandler.hpp>
#include <components/renderingSetup.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>
#include <globals/collisionBits.hpp>

#include <ogl/renderingHelpers.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/gameHelpers.hpp>
#include <tools/b2Helpers.hpp>

#include <algorithm>

namespace Levels
{
	class Rim::Impl
	{
	public:
		void setGraphicsSettings()
		{
			Globals::Components().graphicsSettings().defaultColor = { 0.7f, 0.7f, 0.7f, 1.0f };
			Globals::Components().mainFramebufferRenderer().renderer = Tools::StandardFullscreenRenderer(Globals::Shaders().textured());

			sceneCoordTexturesRS = Globals::Components().renderingSetups().size();
			Globals::Components().renderingSetups().emplace_back([
				sceneCoordTextures = Uniforms::Uniform1b()
			](Shaders::ProgramId program) mutable {
				if (!sceneCoordTextures.isValid())
					sceneCoordTextures = Uniforms::Uniform1b(program, "sceneCoordTextures");

				sceneCoordTextures(true);

				return [=]() mutable { sceneCoordTextures(false); };
			});
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
			textures.back().scale = glm::vec2(20.0f, 20.0f);

			flame1AnimationTexture = textures.size();
			textures.emplace_back("textures/flame animation 1.jpg");
			textures.back().minFilter = GL_LINEAR;

			missileTexture = textures.size();
			textures.emplace_back("textures/missile2.png");
			textures.back().minFilter = GL_LINEAR;
			textures.back().scale = glm::vec2(0.4f, 0.45f);

			explosionTexture = textures.size();
			textures.emplace_back("textures/explosion.png");

			mosaicTexture = textures.size();
			textures.emplace_back("textures/mosaic.jpg", GL_MIRRORED_REPEAT);
			textures.back().scale = glm::vec2(20.0f, 20.0f);
		}

		void setAnimations()
		{
			flame1AnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().push_back(Components::AnimatedTexture(
				flame1AnimationTexture, { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
				AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal));
			Globals::Components().animatedTextures().back().start(true);
		}

		void createMovableWalls()
		{
			auto& walls = Globals::Components().walls();

			const float rimRadius = 15.0f;
			const float rimHThickness = 1.0f;
			const int rimSegments = 50;
			const float rimSegmentMariginsHLength = 0.4f;
			
			constexpr float rimStep = glm::two_pi<float>() / rimSegments;
			const float rimSegmentHLength = glm::distance(glm::vec2(rimRadius, 0.0f),
				glm::vec2(glm::cos(rimStep), glm::sin(rimStep)) * rimRadius) / 2.0f;

			rimWallBegin = walls.size();
			for (int i = 0; i < rimSegments; ++i)
			{
				const glm::vec2 pos1(glm::cos(i * rimStep) * rimRadius, glm::sin(i * rimStep) * rimRadius);
				const glm::vec2 pos2(glm::cos((i + 1) * rimStep) * rimRadius, glm::sin((i + 1) * rimStep) * rimRadius);
				walls.emplace_back(Tools::CreateBoxBody((pos1 + pos2) / 2.0f, { rimHThickness, rimSegmentHLength + rimSegmentMariginsHLength },
					rimStep * (2 * i + 1) / 2, b2_dynamicBody, 0.01f), TCM::Texture(mosaicTexture));
				walls.back().texCoord = walls.back().getTexCoord(true);

				if (i > 0)
					Tools::CreateRevoluteJoint(*(walls.rbegin() + 1)->body, *walls.back().body, pos1);
			}
			rimWallEnd = walls.size();

			Tools::CreateRevoluteJoint(*(walls.rbegin() + rimSegments - 1)->body, *walls.back().body, glm::vec2(rimRadius, 0.0f));
		}

		void createStationaryWalls() const
		{
			auto& walls = Globals::Components().walls();

			const glm::vec2 borderHSize = { 50.0f, 50.0f };
			const float borderHThickness = 2.0f;

			for (int sign : {-1, 1})
			{
				walls.emplace_back(Tools::CreateBoxBody({ borderHSize.x * sign, 0.0f },
					{ borderHThickness, borderHSize.y + borderHThickness }), TCM::Texture(spaceRockTexture), sceneCoordTexturesRS);
				walls.emplace_back(Tools::CreateBoxBody({ 0.0f, borderHSize.y * sign },
					{ borderHSize.x + borderHThickness, borderHThickness }), TCM::Texture(spaceRockTexture), sceneCoordTexturesRS);
			}
		}

		void createPlayers()
		{
			player1Handler = Tools::CreatePlane(rocketPlaneTexture, flame1AnimatedTexture, { 0.0f, 0.0f }, glm::half_pi<float>());
		}

		void setCamera() const
		{
			const auto& plane = Globals::Components().planes()[player1Handler.planeId];

			Globals::Components().camera().targetProjectionHSizeF = [&]() {
				Globals::Components().camera().projectionTransitionFactor = Globals::Components().physics().frameDuration * 6;
				return projectionHSizeBase + glm::length(plane.getVelocity()) * 0.2f;
			};
			Globals::Components().camera().targetPositionF = [&]() {
				Globals::Components().camera().positionTransitionFactor = Globals::Components().physics().frameDuration * 6;
				return plane.getCenter() + glm::vec2(glm::cos(plane.getAngle()), glm::sin(plane.getAngle())) * 5.0f + plane.getVelocity() * 0.4f;
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
						Tools::CreateExplosion(Tools::ExplosionParams().center(ToVec2<glm::vec2>(missileBody.GetWorldCenter())).explosionTexture(explosionTexture));
					}
				}
				});
		}

		void launchMissile()
		{
			auto missileHandler = Tools::CreateMissile(Globals::Components().planes()[player1Handler.planeId].getCenter(),
				Globals::Components().planes()[player1Handler.planeId].getAngle(), 5.0f, { 0.0f, 0.0f }, Globals::Components().planes()[player1Handler.planeId].getVelocity(),
				missileTexture, flame1AnimatedTexture);
			missilesToHandlers.emplace(missileHandler.missileId, std::move(missileHandler));
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

			if (mouseState.pressing.mmb)
				Globals::Components().physics().gameSpeed = std::clamp(Globals::Components().physics().gameSpeed
					+ mouseState.pressed.wheel * 0.1f, 0.0f, 2.0f);
			else
				projectionHSizeBase = std::clamp(projectionHSizeBase + mouseState.pressed.wheel * -5.0f, 5.0f, 100.0f);

			if (mouseState.pressed.xmb1)
				for (unsigned i = rimWallBegin; i < rimWallEnd; ++i)
				{
					auto& renderingSetup = Globals::Components().walls()[i].renderingSetup;
					renderingSetup = renderingSetup
						? std::nullopt
						: std::optional(sceneCoordTexturesRS);
				}
		}

	private:
		unsigned sceneCoordTexturesRS = 0;

		unsigned rocketPlaneTexture = 0;
		unsigned spaceRockTexture = 0;
		unsigned flame1AnimationTexture = 0;
		unsigned missileTexture = 0;
		unsigned explosionTexture = 0;
		unsigned mosaicTexture = 0;

		unsigned flame1AnimatedTexture = 0;

		unsigned rimWallBegin = 0;
		unsigned rimWallEnd = 0;

		Tools::PlaneHandler player1Handler;
		std::unordered_map<ComponentId, Tools::MissileHandler> missilesToHandlers;

		float durationToLaunchMissile = 0.0f;
		float projectionHSizeBase = 20.0f;
	};

	Rim::Rim():
		impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->setAnimations();
		impl->createMovableWalls();
		impl->createStationaryWalls();
		impl->createPlayers();
		impl->setCamera();
		impl->setCollisionCallbacks();
	}

	Rim::~Rim() = default;

	void Rim::step()
	{
		impl->step();
	}
}
