#include "stateController.hpp"

#include "tools/utility.hpp"

#include <components/plane.hpp>
#include <components/grapple.hpp>
#include <components/screenInfo.hpp>
#include <components/keyboard.hpp>
#include <components/mouse.hpp>
#include <components/graphicsSettings.hpp>
#include <components/framebuffers.hpp>
#include <components/texture.hpp>
#include <components/functor.hpp>
#include <components/physics.hpp>
#include <components/gamepad.hpp>

#include <ogl/shaders/textured.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <SDL_events.h>
#include <SDL_gamecontroller.h>

#include <algorithm>

namespace
{
	constexpr int lowerResDivisor = 2;
	constexpr int lowestResDivisor = 4;
	constexpr int pixelArtShorterDim = 100;
	constexpr int lowPixelArtShorterDim = 30;
}

namespace Systems
{
	StateController::StateController() = default;
	StateController::~StateController() = default;

	void StateController::postInit() const
	{
		for(auto& plane: Globals::Components().planes())
			plane.details.previousCenter = plane.getCenter();

		for (auto& [id, grapple] : Globals::Components().grapples())
			grapple.details.previousCenter = grapple.getCenter();
	}

	void StateController::stepSetup() const
	{
		for (auto& [id, stepSetup] : Globals::Components().frameSetups())
			stepSetup();
	}

	void StateController::renderSetup() const
	{
		Globals::Shaders().textured().numOfPlayers(Globals::Components().planes().size());

		for (int i = 0; i < (int)Globals::Components().planes().size(); ++i)
			Globals::Shaders().textured().playersCenter(i, Globals::Components().planes()[i].getCenter());
	}

	void StateController::stepTeardown() const
	{
		for(auto& player: Globals::Components().planes())
			player.details.previousCenter = player.getCenter();

		for (auto& [id, grapple] : Globals::Components().grapples())
			grapple.details.previousCenter = grapple.getCenter();

		for (auto& [id, stepTeardown] : Globals::Components().frameTeardowns())
			stepTeardown();
	}

	void StateController::changeWindowSize(glm::ivec2 size) const
	{
		auto& screenInfo = Globals::Components().screenInfo();
		auto& framebuffers = Globals::Components().framebuffers();

		screenInfo.windowSize = size;
		screenInfo.windowCenterInScreenSpace = { screenInfo.windowLocation + screenInfo.windowSize / 2 };

		auto setTextureFramebufferSize = [&](Components::Framebuffers::SubBuffers& subBuffers, glm::ivec2 size)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, subBuffers.fbo);

			glBindTexture(GL_TEXTURE_2D, subBuffers.textureObject);
			subBuffers.size = size;
			Globals::Components().textures()[subBuffers.textureUnit - GL_TEXTURE0].loaded.size = subBuffers.size;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, subBuffers.size.x, subBuffers.size.y, 0, GL_RGBA, GL_FLOAT, nullptr);

			glBindRenderbuffer(GL_RENDERBUFFER, subBuffers.depthBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);

			assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		};

		const glm::ivec2 pixelArtTextureFramebufferSize = size.x > size.y
			? glm::ivec2((int)(pixelArtShorterDim * (float)size.x / size.y), pixelArtShorterDim)
			: glm::ivec2(pixelArtShorterDim, (int)(pixelArtShorterDim * (float)size.y / size.x));

		const glm::ivec2 lowPixelArtTextureFramebufferSize = size.x > size.y
			? glm::ivec2((int)(lowPixelArtShorterDim * (float)size.x / size.y), lowPixelArtShorterDim)
			: glm::ivec2(lowPixelArtShorterDim, (int)(lowPixelArtShorterDim * (float)size.y / size.x));

		setTextureFramebufferSize(framebuffers.main, size);
		setTextureFramebufferSize(framebuffers.lowerLinearBlend0, size / lowerResDivisor);
		setTextureFramebufferSize(framebuffers.lowerLinearBlend1, size / lowerResDivisor);
		setTextureFramebufferSize(framebuffers.lowestLinearBlend0, size / lowestResDivisor);
		setTextureFramebufferSize(framebuffers.lowestLinearBlend1, size / lowestResDivisor);
		setTextureFramebufferSize(framebuffers.pixelArtBlend0, pixelArtTextureFramebufferSize);
		setTextureFramebufferSize(framebuffers.pixelArtBlend1, pixelArtTextureFramebufferSize);
		setTextureFramebufferSize(framebuffers.lowPixelArtBlend0, lowPixelArtTextureFramebufferSize);
		setTextureFramebufferSize(framebuffers.lowPixelArtBlend1, lowPixelArtTextureFramebufferSize);
	}

	void StateController::changeWindowLocation(glm::ivec2 location) const
	{
		auto& screenInfo = Globals::Components().screenInfo();

		screenInfo.windowLocation = location;
		screenInfo.windowCenterInScreenSpace = { location + screenInfo.windowSize / 2 };
	}

	void StateController::setWindowFocus() const
	{
		resetMousePosition();
		Globals::Components().mouse().pressing = Components::Mouse::Buttons();
		Globals::Components().keyboard().pressing = std::array<bool, 256>({});
		Globals::Components().physics().prevFrameTime = std::chrono::high_resolution_clock::now();
	}

	void StateController::killWindowFocus() const
	{
		Globals::Components().physics().paused = true;
	}

	void StateController::resetMousePosition() const
	{
		Tools::SetMousePos(Globals::Components().screenInfo().windowCenterInScreenSpace);
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
		if (keys['P'] && !prevKeyboardKeys['P'])
			Globals::Components().physics().paused = !Globals::Components().physics().paused;

		auto& keyboard = Globals::Components().keyboard();
		for (size_t i = 0; i < keys.size(); ++i)
		{
			keyboard.pressing[i] = keys[i];
			keyboard.pressed[i] = keys[i] && !prevKeyboardKeys[i];
			keyboard.released[i] = !keys[i] && prevKeyboardKeys[i];
		}

		prevKeyboardKeys = keys;
	}

	void StateController::handleSDL()
	{
		auto getJoystickInstanceId = [](int joystickId) {
			SDL_GameController* controller = SDL_GameControllerOpen(joystickId);
			SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
			return SDL_JoystickInstanceID(joystick);
		};

		auto changeButtonState = [this](int controllerId, int buttonId, bool state) {
			auto& gamepads = Globals::Components().gamepads();
			auto it = controllersToComponents.find(controllerId);
			if (it == controllersToComponents.end())
				return;

			switch (buttonId)
			{
			case SDL_CONTROLLER_BUTTON_A:
				gamepads[it->second].aButton = state;
				break;
			case SDL_CONTROLLER_BUTTON_B:
				gamepads[it->second].bButton = state;
				break;
			case SDL_CONTROLLER_BUTTON_X:
				gamepads[it->second].xButton = state;
				break;
			case SDL_CONTROLLER_BUTTON_Y:
				gamepads[it->second].yButton = state;
				break;
			case SDL_CONTROLLER_BUTTON_BACK:
				gamepads[it->second].backButton = state;
				break;
			case SDL_CONTROLLER_BUTTON_START:
				gamepads[it->second].startButton = state;
				break;
			case SDL_CONTROLLER_BUTTON_LEFTSTICK:
				gamepads[it->second].lStickButton = state;
				break;
			case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
				gamepads[it->second].rStickButton = state;
				break;
			case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
				gamepads[it->second].lShoulderButton = state;
				break;
			case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
				gamepads[it->second].rShoulderButton = state;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_UP:
				gamepads[it->second].dUpButton = state;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
				gamepads[it->second].dDownButton = state;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
				gamepads[it->second].dLeftButton = state;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
				gamepads[it->second].dRightButton = state;
				break;
			}
		};

		auto changeAxisValue = [this](int controllerId, int axisId, int value) {
			auto& gamepads = Globals::Components().gamepads();
			auto it = controllersToComponents.find(controllerId);
			if (it == controllersToComponents.end())
				return;

			value += 32768;

			switch (axisId)
			{
			case SDL_CONTROLLER_AXIS_LEFTX:
				gamepads[it->second].lAxis.x = value / 65535.0f - 0.5f;
				break;
			case SDL_CONTROLLER_AXIS_LEFTY:
				gamepads[it->second].lAxis.y = value / 65535.0f - 0.5f;
				break;
			case SDL_CONTROLLER_AXIS_RIGHTX:
				gamepads[it->second].rAxis.x = value / 65535.0f - 0.5f;
				break;
			case SDL_CONTROLLER_AXIS_RIGHTY:
				gamepads[it->second].rAxis.y = value / 65535.0f - 0.5f;
				break;
			case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
				gamepads[it->second].lTrigger = value / 65535.0f - 0.5f;
				break;
			case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
				gamepads[it->second].rTrigger = value / 65535.0f - 0.5f;
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
	}
}
