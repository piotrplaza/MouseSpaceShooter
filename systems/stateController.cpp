#include "stateController.hpp"

#include "tools/utility.hpp"

#include <components/systemInfo.hpp>
#include <components/keyboard.hpp>
#include <components/mouse.hpp>
#include <components/graphicsSettings.hpp>
#include <components/framebuffers.hpp>
#include <components/texture.hpp>
#include <components/functor.hpp>
#include <components/physics.hpp>
#include <components/gamepad.hpp>
#include <components/pauseHandler.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <SDL_events.h>
#include <SDL_gamecontroller.h>

#include <algorithm>

namespace
{
	void ProcessFunctors(DynamicOrderedComponents<Components::Functor>& functors)
	{
		auto it = functors.begin();
		while (it != functors.end())
		{
			if ((*it)())
				++it;
			else
				it = functors.remove(it);
		}
	}
}

namespace Systems
{
	StateController::StateController()
	{
		auto& limits = Globals::Components().systemInfo().limits;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &limits.maxTextureUnits);
		handleSDL();
	}

	StateController::~StateController() = default;

	void StateController::postInit() const
	{
		ProcessFunctors(Globals::Components().postInits());
	}

	void StateController::stepSetup()
	{
		/*if (deferredPause)
		{
			Globals::Components().physics().paused = true;
			deferredPause = false;
		}*/

		ProcessFunctors(Globals::Components().stepSetups());
	}

	void StateController::stepTeardown() const
	{
		ProcessFunctors(Globals::Components().stepTeardowns());
	}

	void StateController::renderSetup() const
	{
		ProcessFunctors(Globals::Components().renderSetups());
	}

	void StateController::renderTeardown() const
	{
		ProcessFunctors(Globals::Components().renderTeardowns());
	}

	void StateController::changeWindowSize(glm::ivec2 size) const
	{
		auto& screenInfo = Globals::Components().systemInfo().screen;
		auto& framebuffers = Globals::Components().framebuffers();

		screenInfo.windowSize = size;
		screenInfo.windowCenterInScreenSpace = { screenInfo.windowLocation + screenInfo.windowSize / 2 };

		auto setTextureFramebufferSize = [&](Components::Framebuffers::SubBuffers& subBuffers, glm::ivec2 size)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, subBuffers.fbo);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, subBuffers.textureObject);
			subBuffers.size = size;
			Globals::Components().staticTextures()[subBuffers.textureId].loaded.size = subBuffers.size;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, subBuffers.size.x, subBuffers.size.y, 0, GL_RGBA, GL_FLOAT, nullptr);

			glBindRenderbuffer(GL_RENDERBUFFER, subBuffers.depthBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);

			assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		};

		const float aspectRatio = screenInfo.getAspectRatio();

		for (size_t scaling = 0; scaling < (size_t)ResolutionMode::Scaling::COUNT; ++scaling)
			for (size_t blending = 0; blending < (size_t)ResolutionMode::Blending::COUNT; ++blending)
			{
				setTextureFramebufferSize(framebuffers.subBuffers[(size_t)ResolutionMode::Resolution::Native][scaling][blending], size);
				setTextureFramebufferSize(framebuffers.subBuffers[(size_t)ResolutionMode::Resolution::HalfNative][scaling][blending], size / 2);
				setTextureFramebufferSize(framebuffers.subBuffers[(size_t)ResolutionMode::Resolution::QuarterNative][scaling][blending], size / 4);
				setTextureFramebufferSize(framebuffers.subBuffers[(size_t)ResolutionMode::Resolution::OctaNative][scaling][blending], size / 8);
				setTextureFramebufferSize(framebuffers.subBuffers[(size_t)ResolutionMode::Resolution::H2160][scaling][blending], { 2160 * aspectRatio, 2160 });
				setTextureFramebufferSize(framebuffers.subBuffers[(size_t)ResolutionMode::Resolution::H1080][scaling][blending], { 1080 * aspectRatio, 1080 });
				setTextureFramebufferSize(framebuffers.subBuffers[(size_t)ResolutionMode::Resolution::H540][scaling][blending], { 540 * aspectRatio, 540 });
				setTextureFramebufferSize(framebuffers.subBuffers[(size_t)ResolutionMode::Resolution::H405][scaling][blending], { 405 * aspectRatio, 405 });
				setTextureFramebufferSize(framebuffers.subBuffers[(size_t)ResolutionMode::Resolution::H270][scaling][blending], { 270 * aspectRatio, 270 });
				setTextureFramebufferSize(framebuffers.subBuffers[(size_t)ResolutionMode::Resolution::H135][scaling][blending], { 135 * aspectRatio, 135 });
				setTextureFramebufferSize(framebuffers.subBuffers[(size_t)ResolutionMode::Resolution::H68][scaling][blending], { 68 * aspectRatio, 68 });
				setTextureFramebufferSize(framebuffers.subBuffers[(size_t)ResolutionMode::Resolution::H34][scaling][blending], { 34 * aspectRatio, 34 });
				setTextureFramebufferSize(framebuffers.subBuffers[(size_t)ResolutionMode::Resolution::H17][scaling][blending], { 17 * aspectRatio, 17 });
			}
	}

	void StateController::changeWindowLocation(glm::ivec2 location) const
	{
		auto& screenInfo = Globals::Components().systemInfo().screen;

		screenInfo.windowLocation = location;
		screenInfo.windowCenterInScreenSpace = { location + screenInfo.windowSize / 2 };
	}

	void StateController::changeRefreshRate(int refreshRate) const
	{
		Globals::Components().systemInfo().screen.refreshRate = refreshRate;
	}

	void StateController::setWindowFocus(bool focus)
	{
		auto& physics = Globals::Components().physics();
		auto& pauseHandler = Globals::Components().pauseHandler();
		if (!focus)
		{
			if (Globals::Components().physics().paused)
				return;
			pauseHandler.handler(physics.paused);
			physics.paused = true;
		}

		if (!physics.paused)
		{
			Tools::SetMouseCursorVisibility(false);
			resetMousePosition();
			return;
		}

		resetMousePosition();
		resetPrevKeys();
		Tools::SetMouseCursorVisibility(true);

		auto& mouse = Globals::Components().mouse();
		mouse.pressing = mouse.pressed = mouse.released = {};

		auto& keyboard = Globals::Components().keyboard();
		keyboard.pressing = keyboard.pressed = keyboard.released = {};

		auto gamepads = Globals::Components().gamepads();
		for (auto& gamepad : gamepads)
			gamepad.pressing = gamepad.pressed = gamepad.released = {};

		Globals::Components().physics().prevFrameTime = std::chrono::high_resolution_clock::now();
	}

	void StateController::resetMousePosition() const
	{
		Tools::SetMousePos(Globals::Components().systemInfo().screen.windowCenterInScreenSpace);
	}

	void StateController::handleMouseButtons()
	{
		auto& mouse = Globals::Components().mouse();

		auto updateButton = [&](bool Components::Mouse::Buttons::* button)
		{
			mouse.pressed.*button = mouse.pressing.*button && !(prevMouseKeys.*button);
			mouse.released.*button = !(mouse.pressing.*button) && prevMouseKeys.*button;
		};

		updateButton(&Components::Mouse::Buttons::lmb);
		updateButton(&Components::Mouse::Buttons::rmb);
		updateButton(&Components::Mouse::Buttons::mmb);
		updateButton(&Components::Mouse::Buttons::xmb1);
		updateButton(&Components::Mouse::Buttons::xmb2);

		mouse.pressed.wheel = mouse.pressing.wheel - prevMouseKeys.wheel;
		mouse.released.wheel = mouse.pressed.wheel;

		prevMouseKeys = mouse.pressing;
	}

	void StateController::handleKeyboard(const std::array<bool, 256>& keys)
	{
		auto& keyboard = Globals::Components().keyboard();
		auto& physics = Globals::Components().physics();
		const auto& pauseHandler = Globals::Components().pauseHandler();

		for (size_t i = 0; i < keys.size(); ++i)
		{
			keyboard.pressing[i] = keys[i];
			keyboard.pressed[i] = keys[i] && !prevKeyboardKeys[i];
			keyboard.released[i] = !keys[i] && prevKeyboardKeys[i];
		}

		prevKeyboardKeys = keys;

		if (keyboard.pressed['P'])
			physics.paused = pauseHandler.handler(physics.paused);
	}

	void StateController::handleSDL()
	{
		auto getJoystickInstanceId = [](int joystickId) {
			SDL_GameController* controller = SDL_GameControllerOpen(joystickId);
			SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
			return SDL_JoystickInstanceID(joystick);
		};

		auto changeButtonState = [this](int controllerId, int buttonId, bool state) {
			auto it = controllersToComponents.find(controllerId);
			if (it == controllersToComponents.end())
				return;

			auto& gamepad = Globals::Components().gamepads()[it->second];

			switch (buttonId)
			{
			case SDL_CONTROLLER_BUTTON_A:
				gamepad.pressing.a = state;
				break;
			case SDL_CONTROLLER_BUTTON_B:
				gamepad.pressing.b = state;
				break;
			case SDL_CONTROLLER_BUTTON_X:
				gamepad.pressing.x = state;
				break;
			case SDL_CONTROLLER_BUTTON_Y:
				gamepad.pressing.y = state;
				break;
			case SDL_CONTROLLER_BUTTON_BACK:
				gamepad.pressing.back = state;
				break;
			case SDL_CONTROLLER_BUTTON_START:
				gamepad.pressing.start = state;
				break;
			case SDL_CONTROLLER_BUTTON_LEFTSTICK:
				gamepad.pressing.lStick = state;
				break;
			case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
				gamepad.pressing.rStick = state;
				break;
			case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
				gamepad.pressing.lShoulder = state;
				break;
			case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
				gamepad.pressing.rShoulder = state;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_UP:
				gamepad.pressing.dUp = state;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
				gamepad.pressing.dDown = state;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
				gamepad.pressing.dLeft = state;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
				gamepad.pressing.dRight = state;
				break;
			}
		};

		auto changeAxisValue = [this](int controllerId, int axisId, int value) {
			auto& gamepads = Globals::Components().gamepads();
			auto it = controllersToComponents.find(controllerId);
			if (it == controllersToComponents.end())
				return;

			switch (axisId)
			{
			case SDL_CONTROLLER_AXIS_LEFTX:
				gamepads[it->second].lStick.x = (value + 32768) / 65535.0f * 2.0f - 1.0f;
				break;
			case SDL_CONTROLLER_AXIS_LEFTY:
				gamepads[it->second].lStick.y = (value + 32768) / 65535.0f * -2.0f + 1.0f;
				break;
			case SDL_CONTROLLER_AXIS_RIGHTX:
				gamepads[it->second].rStick.x = (value + 32768) / 65535.0f * 2.0f - 1.0f;
				break;
			case SDL_CONTROLLER_AXIS_RIGHTY:
				gamepads[it->second].rStick.y = (value + 32768) / 65535.0f * -2.0f + 1.0f;
				break;
			case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
				gamepads[it->second].lTrigger = value / 32767.0f;
				break;
			case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
				gamepads[it->second].rTrigger = value / 32767.0f;
				break;
			}
		};

		SDL_Event event;
		while (SDL_PollEvent(&event) != 0)
		{
			switch (event.cdevice.type)
			{
			case SDL_CONTROLLERDEVICEADDED:
			{
				auto& gamepads = Globals::Components().gamepads();
				const int joystickInstanceId = getJoystickInstanceId(event.cdevice.which);
				for (int i = 0; i < (int)gamepads.size(); ++i)
				{
					auto it = std::find_if(controllersToComponents.begin(), controllersToComponents.end(), [&](const auto& val) {
						return val.second == i;
						});

					if (it != controllersToComponents.end())
						continue;

					controllersToComponents.emplace(joystickInstanceId, i);
					gamepads[i].enabled = true;
					break;
				}
			} break;
			case SDL_CONTROLLERDEVICEREMOVED:
			{
				auto& gamepads = Globals::Components().gamepads();
				auto it = controllersToComponents.find(event.cdevice.which);
				if (it == controllersToComponents.end())
					continue;
				gamepads[it->second] = Components::Gamepad();
				prevGamepadsKeys[it->second] = Components::Gamepad::Buttons();
				controllersToComponents.erase(it);
			} break;
			case SDL_CONTROLLERDEVICEREMAPPED:
				assert(!"remapping not implemented");
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				changeButtonState(event.cbutton.which, event.cbutton.button, true);
				break;
			case SDL_CONTROLLERBUTTONUP:
				changeButtonState(event.cbutton.which, event.cbutton.button, false);
				break;
			case SDL_CONTROLLERAXISMOTION:
				changeAxisValue(event.caxis.which, event.caxis.axis, event.caxis.value);
				break;
			}
		}

		auto updateButton = [&](bool Components::Gamepad::Buttons::* button)
		{
			for (size_t i = 0; i < Globals::Components().gamepads().size(); ++i)
			{
				auto& gamepad = Globals::Components().gamepads()[i];
				gamepad.pressed.*button = gamepad.pressing.*button && !(prevGamepadsKeys[i].*button);
				gamepad.released.*button = !(gamepad.pressing.*button) && prevGamepadsKeys[i].*button;
				prevGamepadsKeys[i].*button = gamepad.pressing.*button;
			}
		};

		updateButton(&Components::Gamepad::Buttons::a);
		updateButton(&Components::Gamepad::Buttons::b);
		updateButton(&Components::Gamepad::Buttons::x);
		updateButton(&Components::Gamepad::Buttons::y);
		updateButton(&Components::Gamepad::Buttons::back);
		updateButton(&Components::Gamepad::Buttons::start);
		updateButton(&Components::Gamepad::Buttons::lStick);
		updateButton(&Components::Gamepad::Buttons::rStick);
		updateButton(&Components::Gamepad::Buttons::lShoulder);
		updateButton(&Components::Gamepad::Buttons::rShoulder);
		updateButton(&Components::Gamepad::Buttons::dUp);
		updateButton(&Components::Gamepad::Buttons::dDown);
		updateButton(&Components::Gamepad::Buttons::dLeft);
		updateButton(&Components::Gamepad::Buttons::dRight);

		const auto& pauseHandler = Globals::Components().pauseHandler();
		auto& physics = Globals::Components().physics();

		for (const auto& gamepad : Globals::Components().gamepads())
			if (gamepad.isEnabled() && gamepad.pressed.start)
				physics.paused = pauseHandler.handler(physics.paused);
	}

	void StateController::resetPrevKeys()
	{
		prevKeyboardKeys = {};
		prevMouseKeys = {};
		prevGamepadsKeys = {};
	}
}
