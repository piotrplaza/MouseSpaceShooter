#include "rim.hpp"

#include <components/graphicsSettings.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/camera.hpp>
#include <components/plane.hpp>
#include <components/wall.hpp>
#include <components/decoration.hpp>
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

			recursiveFaceRSBegin = Globals::Components().renderingSetups().size();

			for (unsigned i = 0; i < numOfRecursiveFaces; ++i)
			{
				Globals::Components().renderingSetups().emplace_back([
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
			auto& textures = Globals::Components().textures();

			rocketPlaneTexture = textures.size();
			textures.emplace_back("textures/rocket plane.png");
			textures.back().translate = glm::vec2(0.4f, 0.0f);
			textures.back().scale = glm::vec2(1.6f, 1.8f);

			spaceRockTexture = textures.size();
			textures.emplace_back("textures/space rock.jpg", GL_MIRRORED_REPEAT);
			textures.back().scale = glm::vec2(50.0f, 50.0f);

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

			recursiveFaceAnimationTexture = textures.size();
			textures.emplace_back("textures/recursive face animation.jpg");
			textures.back().minFilter = GL_LINEAR;
		}

		void setAnimations()
		{
			flame1AnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().push_back(Components::AnimatedTexture(
				flame1AnimationTexture, { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, Tools::Random(0.01f, 0.03f), 32, 0,
				AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal));
			Globals::Components().animatedTextures().back().start(true);

			recursiveFaceAnimatedTextureBegin = Globals::Components().animatedTextures().size();

			for (unsigned i = 0; i < numOfRecursiveFaces; ++i)
			{
				Globals::Components().animatedTextures().push_back(Components::AnimatedTexture(
					recursiveFaceAnimationTexture, { 263, 525 }, { 5, 10 }, { 0, 0 }, 210, 473, { 52, 52 }, 0.02f, 50, rand() % 50,
					(i % 2 == 0) ? AnimationDirection::Forward : AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal));
				Globals::Components().animatedTextures().back().start(true);
			}
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
					rimStep * (2 * i + 1) / 2, b2_dynamicBody, 0.01f), TCM::Texture(mosaicTexture), sceneCoordTexturesRS);
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

			for (int sign : {-1, 1})
			{
				walls.emplace_back(Tools::CreateBoxBody({ (borderHSize.x + borderHThickness) * sign, 0.0f },
					{ borderHThickness, borderHSize.y + borderHThickness * 2.0f }), TCM::Texture(spaceRockTexture),
					sceneCoordTexturesRS).colorF = []() { return glm::vec4(0.1f, 0.1f, 0.1f, 1.0f); };
				walls.emplace_back(Tools::CreateBoxBody({ 0.0f, (borderHSize.y + borderHThickness) * sign },
					{ borderHSize.x + borderHThickness * 2.0f, borderHThickness }), TCM::Texture(spaceRockTexture),
					sceneCoordTexturesRS).colorF = []() { return glm::vec4(0.1f, 0.1f, 0.1f, 1.0f); };
			}
		}

		void createAdditionalDecorations()
		{
			auto& decorations = Globals::Components().decorations();

			for (unsigned i = 0; i < numOfRecursiveFaces; ++i)
			{
				decorations.emplace_back(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 1.0f, 1.0f }),
					TCM::AnimatedTexture(recursiveFaceAnimatedTextureBegin + i), Tools::CreateTexCoordOfRectangle(), recursiveFaceRSBegin + i, RenderLayer::NearBackground);
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
						Tools::CreateExplosion(Tools::ExplosionParams().center(ToVec2<glm::vec2>(missileBody.GetWorldCenter())).explosionTexture(explosionTexture)
							.particlesRadius(0.5f).particlesDensity(0.02f));
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
			const auto& mouse = Globals::Components().mouse();
			auto& player1Controls = Globals::Components().planes()[player1Handler.planeId].controls;

			player1Controls.turningDelta = mouse.getWorldSpaceDelta();
			player1Controls.autoRotation = mouse.pressing.rmb;
			player1Controls.throttling = (float)mouse.pressing.rmb;
			player1Controls.magneticHook = mouse.pressing.xmb1;

			if (mouse.pressing.lmb)
			{
				if (durationToLaunchMissile <= 0.0f)
				{
					launchMissile();
					durationToLaunchMissile = 0.1f;
				}
				else durationToLaunchMissile -= Globals::Components().physics().frameDuration;
			}
			else durationToLaunchMissile = 0.0f;

			if (mouse.pressing.mmb)
				Globals::Components().physics().gameSpeed = std::clamp(Globals::Components().physics().gameSpeed
					+ mouse.pressed.wheel * 0.1f, 0.0f, 2.0f);
			else
				projectionHSizeBase = std::clamp(projectionHSizeBase + mouse.pressed.wheel * -5.0f, 5.0f, 100.0f);

			if (mouse.pressed.xmb1)
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
		unsigned recursiveFaceRSBegin = 0;

		unsigned rocketPlaneTexture = 0;
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
