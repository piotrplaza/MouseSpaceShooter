#include "rocketball.hpp"

#include <components/screenInfo.hpp>
#include <components/physics.hpp>
#include <components/texture.hpp>
#include <components/plane.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/camera.hpp>
#include <components/decoration.hpp>
#include <components/graphicsSettings.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>
#include <components/animatedTexture.hpp>

#include <globals/components.hpp>

#include <ogl/uniforms.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>

#include <algorithm>

namespace Levels
{
	class Rocketball::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			Globals::Components().graphicsSettings().clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			Globals::Components().graphicsSettings().defaultColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().textures();

			plane1Texture = textures.size();
			textures.emplace("textures/plane 1.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.6f, 1.8f);
			textures.last().minFilter = GL_LINEAR;

			woodTexture = textures.size();
			textures.emplace("textures/wood.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(50.0f);

			orbTexture = textures.size();
			textures.emplace("textures/orb.png");
			textures.last().scale = glm::vec2(4.0f);

			playFieldTexture = textures.size();
			textures.emplace("textures/play field.jpg");

			flame1AnimationTexture = textures.size();
			textures.emplace("textures/flame animation 1.jpg");
			textures.last().minFilter = GL_LINEAR;
		}

		void setAnimations()
		{
			flame1AnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().add({ flame1AnimationTexture, { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
				AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal });
			Globals::Components().animatedTextures().last().start(true);
		}

		void createBackground() const
		{
			Globals::Components().staticDecorations().emplace(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 100.0f, 60.0f }),
				TCM::Texture(playFieldTexture), Tools::CreateTexCoordOfRectangle(), std::nullopt, RenderLayer::Background);
		}

		void createPlayers()
		{
			player1Id = Tools::CreatePlane(Tools::CreateTrianglesBody({ { glm::vec2{2.0f, 0.0f}, glm::vec2{-1.0f, 1.0f}, glm::vec2{-1.0f, -1.0f} } }, Tools::GetDefaultParamsForPlaneBody()),
				plane1Texture, flame1AnimatedTexture, Tools::PlaneParams().position({ -10.0f, 0.0f }));
			Globals::Components().planes()[player1Id].connectIfApproaching = true;
		}

		void createStationaryWalls() const
		{
			const glm::vec2 levelHSize = { 100.0f, 60.0f };
			const float bordersHGauge = 50.0f;

			Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ bordersHGauge, levelHSize.y + bordersHGauge * 2 },
				Tools::BodyParams().position({ -levelHSize.x - bordersHGauge, 0.0f })), TCM::Texture(woodTexture));

			Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ bordersHGauge, levelHSize.y + bordersHGauge * 2 },
				Tools::BodyParams().position({ levelHSize.x + bordersHGauge, 0.0f })), TCM::Texture(woodTexture));

			Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ levelHSize.x + bordersHGauge * 2, bordersHGauge },
				Tools::BodyParams().position({ 0.0f, -levelHSize.y - bordersHGauge })), TCM::Texture(woodTexture));

			Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ levelHSize.x + bordersHGauge * 2, bordersHGauge },
				Tools::BodyParams().position({ 0.0f, levelHSize.y + bordersHGauge })), TCM::Texture(woodTexture));
		}

		void createGrapples()
		{
			ball = &Globals::Components().grapples().emplace(Tools::CreateCircleBody(2.0f,
				Tools::BodyParams().bodyType(b2_dynamicBody).density(0.02f).restitution(0.5f)), TCM::Texture(orbTexture));
			ball->influenceRadius = 15.0f;
		}

		void setCamera() const
		{
			const auto& player = Globals::Components().planes()[player1Id];

			Globals::Components().camera().targetProjectionHSizeF = [&]() {
				Globals::Components().camera().projectionTransitionFactor = Globals::Components().physics().frameDuration * 6;
				return 30.0f + glm::distance(player.getCenter(), ball->getCenter()) * 0.3f;
			};
			Globals::Components().camera().targetPositionF = [&]() {
				Globals::Components().camera().positionTransitionFactor = Globals::Components().physics().frameDuration * 6;
				return (player.getCenter() + ball->getCenter()) * 0.5f;
			};
		}

		void step()
		{
			float mouseSensitivity = 0.01f;
			float gamepadSensitivity = 50.0f;

			const auto& physics = Globals::Components().physics();
			const auto& mouse = Globals::Components().mouse();
			const auto& gamepad = Globals::Components().gamepads()[0];
			auto& player1Controls = Globals::Components().planes()[player1Id].controls;

			player1Controls.turningDelta = mouse.getWorldSpaceDelta() * mouseSensitivity +
				Tools::ApplyDeadzone(gamepad.lStick) * physics.frameDuration * gamepadSensitivity;
			player1Controls.autoRotation = (bool)std::max((float)mouse.pressing.rmb, gamepad.rTrigger);
			player1Controls.throttling = std::max((float)mouse.pressing.rmb, gamepad.rTrigger);
			player1Controls.magneticHook = mouse.pressing.xmb1 || gamepad.pressing.lShoulder || gamepad.lTrigger >= 0.5f;
		}

	private:
		unsigned plane1Texture = 0;
		unsigned woodTexture = 0;
		unsigned orbTexture = 0;
		unsigned playFieldTexture = 0;
		unsigned flame1AnimationTexture = 0;

		unsigned flame1AnimatedTexture = 0;

		unsigned player1Id;
		Components::Grapple* ball = nullptr;
	};

	Rocketball::Rocketball():
		impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->setAnimations();
		impl->createBackground();
		impl->createPlayers();
		impl->createStationaryWalls();
		impl->createGrapples();
		impl->setCamera();
	}

	Rocketball::~Rocketball() = default;

	void Rocketball::step()
	{
		impl->step();
	}
}
