#include "basic.hpp"

#include <components/graphicsSettings.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/camera.hpp>
#include <components/plane.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <ogl/renderingHelpers.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/gameHelpers.hpp>

#include <iostream>
using namespace std;

namespace Levels
{
	class Basic::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			Globals::Components().graphicsSettings().defaultColor = { 0.7f, 0.7f, 0.7f, 1.0f };
			Globals::Components().mainFramebufferRenderer().renderer = Tools::StandardFullscreenRenderer(Globals::Shaders().textured());
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().textures();

			rocketPlaneTexture = textures.size();
			textures.emplace("textures/rocket plane.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.6f, 1.8f);

			alienSpaceshipTexture = textures.size();
			textures.emplace("textures/alien ship 1.png");
			textures.last().scale = glm::vec2(1.9f);

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

		void createAdditionalDecorations() const
		{
		}

		void createPlayers()
		{
			/*player1Id = Tools::CreatePlane(Tools::CreateTrianglesBody({ { glm::vec2{2.0f, 0.0f}, glm::vec2{-1.0f, 1.0f}, glm::vec2{-1.0f, -1.0f} } }, Tools::GetDefaultParamsForPlaneBody()),
				rocketPlaneTexture, flame1AnimatedTexture, Tools::PlaneParams().angle(glm::half_pi<float>()));*/

			player1Id = Tools::CreatePlane(Tools::CreateCircleBody(0.978f, Tools::GetDefaultParamsForPlaneBody()),
				alienSpaceshipTexture, flame1AnimatedTexture, Tools::PlaneParams().angle(glm::half_pi<float>()).numOfThrusts(1).thrustOffset(0.0f).thrustAngle(0.0f));

			cout << Globals::Components().planes()[player1Id].body->GetMass() << endl;
		}

		void setCamera() const
		{
			const auto& plane = Globals::Components().planes()[player1Id];

			Globals::Components().camera().targetProjectionHSizeF = [&]() {
				return 10.0f;
			};
			Globals::Components().camera().targetPositionF = [&]() {
				return glm::vec2(0.0f, 0.0f);
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
		unsigned rocketPlaneTexture = 0;
		unsigned alienSpaceshipTexture = 0;
		unsigned flame1AnimationTexture = 0;

		unsigned flame1AnimatedTexture = 0;

		unsigned player1Id = 0;
	};

	Basic::Basic():
		impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->setAnimations();
		impl->createPlayers();
		impl->setCamera();
	}

	Basic::~Basic() = default;

	void Basic::step()
	{
		impl->step();
	}
}
