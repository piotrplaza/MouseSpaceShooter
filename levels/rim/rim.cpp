#include "rim.hpp"

#include <components/graphicsSettings.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/camera2D.hpp>
#include <components/plane.hpp>
#include <components/wall.hpp>
#include <components/decoration.hpp>
#include <components/collisionHandler.hpp>
#include <components/renderingSetup.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>
#include <globals/collisionBits.hpp>

#include <ogl/renderingHelpers.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/gameHelpers.hpp>
#include <tools/b2Helpers.hpp>
#include <tools/shapes2D.hpp>

#include <algorithm>

namespace
{
	constexpr glm::vec2 borderHSize = { 50.0f, 50.0f };
	constexpr float borderHThickness = 10.0f;
	constexpr unsigned numOfRecursiveFaces = 200;
}

namespace Levels
{
	class Rim::Impl
	{
	public:
		void setGraphicsSettings()
		{
			Globals::Components().graphicsSettings().defaultColor = { 0.7f, 0.7f, 0.7f, 1.0f };

			sceneCoordTexturesRS = Globals::Components().renderingSetups().size();
			Globals::Components().renderingSetups().emplace([
				sceneCoordTextures = Uniforms::Uniform1b()
			](Shaders::ProgramId program) mutable {
				if (!sceneCoordTextures.isValid())
					sceneCoordTextures = Uniforms::Uniform1b(program, "sceneCoordTextures");

				sceneCoordTextures(true);

				return [=]() mutable { sceneCoordTextures(false); };
			});

			recursiveFaceRSBegin = Globals::Components().renderingSetups().size();

			for (unsigned i = 0; i < numOfRecursiveFaces; ++i)
			{
				Globals::Components().renderingSetups().emplace([
					modelMatrix = Uniforms::UniformMat4f(),
					color = Uniforms::Uniform4f(),
					visibilityReduction = Uniforms::Uniform1b(),
					visibilityCenter = Uniforms::Uniform2f(),
					fullVisibilityDistance = Uniforms::Uniform1f(),
					invisibilityDistance = Uniforms::Uniform1f(),
					startTime = Globals::Components().physics().simulationDuration,
					cycleDuration = Tools::Random(1.0f, 5.0f),
					scale = Tools::Random(5.0f, 20.0f),
					angle = 0.0f,
					pos = glm::vec2(Tools::Random(-borderHSize.x, borderHSize.x), Tools::Random(-borderHSize.y, borderHSize.y)) * 0.8f,
					rotSpeed = Tools::Random(-5.0f, 5.0f),
					targetColor = glm::vec3(Tools::Random(0.0f, 1.0f), Tools::Random(0.0f, 1.0f), Tools::Random(0.0f, 1.0f))
				](Shaders::ProgramId program) mutable {
					if (!modelMatrix.isValid())
					{
						modelMatrix = Uniforms::UniformMat4f(program, "model");
						color = Uniforms::Uniform4f(program, "color");
						visibilityReduction = Uniforms::Uniform1b(program, "visibilityReduction");
						visibilityCenter = Uniforms::Uniform2f(program, "visibilityCenter");
						fullVisibilityDistance = Uniforms::Uniform1f(program, "fullVisibilityDistance");
						invisibilityDistance = Uniforms::Uniform1f(program, "invisibilityDistance");
					}

					float cycleTime = Globals::Components().physics().simulationDuration - startTime;

					if (cycleTime > cycleDuration)
					{
						cycleTime = 0.0f;
						cycleDuration = Tools::Random(1.0f, 5.0f);
						scale = Tools::Random(5.0f, 20.0f);
						pos = glm::vec2(Tools::Random(-borderHSize.x, borderHSize.x), Tools::Random(-borderHSize.y, borderHSize.y)) * 0.8f;
						rotSpeed = Tools::Random(-5.0f, 5.0f);
						targetColor = glm::vec3(Tools::Random(0.0f, 1.0f), Tools::Random(0.0f, 1.0f), Tools::Random(0.0f, 1.0f));

						startTime = Globals::Components().physics().simulationDuration;
					}

					modelMatrix(glm::scale(
						glm::rotate(
							glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0)),
							angle += Globals::Components().physics().frameDuration * rotSpeed, { 0.0f, 0.0f, -1.0f }),
						{ scale, scale, 1.0f }));
					color(glm::vec4(targetColor, 1.0f) * glm::sin(cycleTime / cycleDuration * glm::pi<float>()) * 0.2f);
					visibilityReduction(true);
					visibilityCenter(pos);
					fullVisibilityDistance(0.0f);
					invisibilityDistance(scale * 0.8f);

					return [=]() mutable {
						color(glm::vec4(1.0f));
						visibilityReduction(false);
					};
				});
			}
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().staticTextures();

			plane1Texture = textures.size();
			textures.emplace("textures/plane 1.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.6f, 1.8f);
			textures.last().minFilter = GL_LINEAR;

			spaceRockTexture = textures.size();
			textures.emplace("textures/space rock.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(50.0f, 50.0f);

			flame1AnimationTexture = textures.size();
			textures.emplace("textures/flame animation 1.jpg");
			textures.last().minFilter = GL_LINEAR;

			missileTexture = textures.size();
			textures.emplace("textures/missile 2.png");
			textures.last().minFilter = GL_LINEAR;
			textures.last().scale = glm::vec2(0.4f, 0.45f);

			explosionTexture = textures.size();
			textures.emplace("textures/explosion.png");

			mosaicTexture = textures.size();
			textures.emplace("textures/mosaic.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(20.0f, 20.0f);

			recursiveFaceAnimationTexture = textures.size();
			textures.emplace("textures/recursive face animation.jpg");
			textures.last().minFilter = GL_LINEAR;
		}

		void setAnimations()
		{
			flame1AnimatedTexture = Globals::Components().staticAnimatedTextures().size();
			Globals::Components().staticAnimatedTextures().add({ TCM::StaticTexture(flame1AnimationTexture), { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, Tools::Random(0.01f, 0.03f), 32, 0,
				AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal });
			Globals::Components().staticAnimatedTextures().last().start(true);

			recursiveFaceAnimatedTextureBegin = Globals::Components().staticAnimatedTextures().size();

			for (unsigned i = 0; i < numOfRecursiveFaces; ++i)
			{
				Globals::Components().staticAnimatedTextures().add({ TCM::StaticTexture(recursiveFaceAnimationTexture), { 263, 525 }, { 5, 10 }, { 0, 0 }, 210, 473, { 52, 52 }, 0.02f, 50, rand() % 50,
					(i % 2 == 0) ? AnimationDirection::Forward : AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal });
				Globals::Components().staticAnimatedTextures().last().start(true);
			}
		}

		void createMovableWalls()
		{
			auto& staticWalls = Globals::Components().staticWalls();

			const float rimRadius = 15.0f;
			const float rimHThickness = 1.0f;
			const int rimSegments = 50;
			const float rimSegmentMariginsHLength = 0.4f;
			
			constexpr float rimStep = glm::two_pi<float>() / rimSegments;
			const float rimSegmentHLength = glm::distance(glm::vec2(rimRadius, 0.0f),
				glm::vec2(glm::cos(rimStep), glm::sin(rimStep)) * rimRadius) / 2.0f;

			rimWallBegin = staticWalls.size();
			for (int i = 0; i < rimSegments; ++i)
			{
				const glm::vec2 pos1(glm::cos(i * rimStep) * rimRadius, glm::sin(i * rimStep) * rimRadius);
				const glm::vec2 pos2(glm::cos((i + 1) * rimStep) * rimRadius, glm::sin((i + 1) * rimStep) * rimRadius);
				staticWalls.emplace(Tools::CreateBoxBody({ rimHThickness, rimSegmentHLength + rimSegmentMariginsHLength },
					Tools::BodyParams().position((pos1 + pos2) / 2.0f).angle(rimStep * (2 * i + 1) / 2).bodyType(b2_dynamicBody).density(0.01f)), TCM::StaticTexture(mosaicTexture), sceneCoordTexturesRS);
				staticWalls.last().texCoord = staticWalls.last().getTexCoords(true);

				if (i > 0)
					Tools::CreateRevoluteJoint(*(staticWalls.rbegin() + 1)->body, *staticWalls.last().body, pos1);
			}
			rimWallEnd = staticWalls.size();

			Tools::CreateRevoluteJoint(*(staticWalls.rbegin() + rimSegments - 1)->body, *staticWalls.last().body, glm::vec2(rimRadius, 0.0f));
		}

		void createStationaryWalls() const
		{
			auto& staticWalls = Globals::Components().staticWalls();

			for (int sign : {-1, 1})
			{
				staticWalls.emplace(Tools::CreateBoxBody({ borderHThickness, borderHSize.y + borderHThickness * 2.0f },
					Tools::BodyParams().position({ (borderHSize.x + borderHThickness) * sign, 0.0f })), TCM::StaticTexture(spaceRockTexture),
					sceneCoordTexturesRS).colorF = []() { return glm::vec4(0.1f, 0.1f, 0.1f, 1.0f); };
				staticWalls.emplace(Tools::CreateBoxBody({ borderHSize.x + borderHThickness * 2.0f, borderHThickness },
					Tools::BodyParams().position({ 0.0f, (borderHSize.y + borderHThickness) * sign })), TCM::StaticTexture(spaceRockTexture),
					sceneCoordTexturesRS).colorF = []() { return glm::vec4(0.1f, 0.1f, 0.1f, 1.0f); };
			}
		}

		void createAdditionalDecorations() const
		{
			auto& staticDecorations = Globals::Components().staticDecorations();

			for (unsigned i = 0; i < numOfRecursiveFaces; ++i)
			{
				staticDecorations.emplace(Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 1.0f, 1.0f }),
					TCM::StaticAnimatedTexture(recursiveFaceAnimatedTextureBegin + i), Shapes2D::CreateTexCoordOfRectangle(), recursiveFaceRSBegin + i, RenderLayer::NearBackground);
			}
		}

		void createPlayers()
		{
			player1Id = Tools::CreatePlane(Tools::CreateTrianglesBody({ { glm::vec2{2.0f, 0.0f}, glm::vec2{-1.0f, 1.0f}, glm::vec2{-1.0f, -1.0f} } }, Tools::GetDefaultParamsForPlaneBody()),
				plane1Texture, flame1AnimatedTexture, Tools::PlaneParams().angle(glm::half_pi<float>()));
		}

		void setCamera() const
		{
			const auto& plane = Globals::Components().planes()[player1Id];

			Globals::Components().camera2D().projectionTransitionFactor = 6;
			Globals::Components().camera2D().targetProjectionHSizeF = [&]() {
				return projectionHSizeBase + glm::length(plane.getVelocity()) * 0.2f;
			};
			Globals::Components().camera2D().positionTransitionFactor = 6;
			Globals::Components().camera2D().targetPositionF = [&]() {
				return plane.getOrigin2D() + glm::vec2(glm::cos(plane.getAngle()), glm::sin(plane.getAngle())) * 5.0f + plane.getVelocity() * 0.4f;
			};
		}

		void setCollisionCallbacks()
		{
			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::missile, Globals::CollisionBits::all,
				[this](const auto& fixtureA, const auto& fixtureB) {
					for (const auto* fixture : { &fixtureA, &fixtureB })
					if (fixture->GetFilterData().categoryBits == Globals::CollisionBits::missile)
					{
						const auto& targetFixture = fixture == &fixtureA ? fixtureB : fixtureA;
						const auto& missileBody = *fixture->GetBody();
						missilesToHandlers.erase(std::get<TCM::Missile>(Tools::AccessUserData(missileBody).bodyComponentVariant).id);
						Tools::CreateExplosion(Tools::ExplosionParams().center(ToVec2<glm::vec2>(missileBody.GetWorldCenter())).explosionTexture(explosionTexture)
							.particlesRadius(0.5f).particlesDensity(0.02f));
					}
				});
		}

		void launchMissile()
		{
			auto missileHandler = Tools::CreateMissile(Globals::Components().planes()[player1Id].getOrigin2D(),
				Globals::Components().planes()[player1Id].getAngle(), 5.0f, { 0.0f, 0.0f }, Globals::Components().planes()[player1Id].getVelocity(),
				missileTexture, flame1AnimatedTexture);
			missilesToHandlers.emplace(missileHandler.missileId, std::move(missileHandler));
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
			player1Controls.magneticHook = mouse.pressing.xmb1 || gamepad.pressing.lShoulder || gamepad.lTrigger >= 0.5f;

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

			if (mouse.pressing.mmb || gamepad.pressing.rShoulder)
				Globals::Components().physics().gameSpeed = std::clamp(Globals::Components().physics().gameSpeed +
					(mouse.pressed.wheel + gamepad.pressed.dUp * 1 + gamepad.pressed.dDown * -1) * 0.1f, 0.0f, 2.0f);
			else
				projectionHSizeBase = std::clamp(projectionHSizeBase + (mouse.pressed.wheel +
					gamepad.pressed.dUp * 1 + gamepad.pressed.dDown * -1) * -5.0f, 5.0f, 100.0f);

			if (mouse.pressed.xmb1 || gamepad.pressed.lShoulder)
				for (unsigned i = rimWallBegin; i < rimWallEnd; ++i)
				{
					auto& renderingSetup = Globals::Components().staticWalls()[i].renderingSetup;
					renderingSetup = renderingSetup
						? std::nullopt
						: std::optional(sceneCoordTexturesRS);
				}
		}

	private:
		unsigned sceneCoordTexturesRS = 0;
		unsigned recursiveFaceRSBegin = 0;

		unsigned plane1Texture = 0;
		unsigned spaceRockTexture = 0;
		unsigned flame1AnimationTexture = 0;
		unsigned missileTexture = 0;
		unsigned explosionTexture = 0;
		unsigned mosaicTexture = 0;
		unsigned recursiveFaceAnimationTexture = 0;

		unsigned flame1AnimatedTexture = 0;
		unsigned recursiveFaceAnimatedTextureBegin = 0;

		unsigned rimWallBegin = 0;
		unsigned rimWallEnd = 0;

		unsigned player1Id{};
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
		impl->createPlayers();
		impl->createMovableWalls();
		impl->createStationaryWalls();
		impl->createAdditionalDecorations();
		impl->setCamera();
		impl->setCollisionCallbacks();
	}

	Rim::~Rim() = default;

	void Rim::step()
	{
		impl->step();
	}
}
