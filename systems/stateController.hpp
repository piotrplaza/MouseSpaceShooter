#pragma once

#include <components/mouse.hpp>
#include <components/gamepad.hpp>

#include <glm/vec2.hpp>

#include <array>
#include <unordered_map>

namespace Systems
{
	class StateController
	{
	public:
		StateController();
		~StateController();

		void postInit() const;
		void stepSetup();
		void stepTeardown() const;
		void renderSetup() const;
		void renderTeardown() const;
		void changeWindowSize(glm::ivec2 size) const;
		void changeWindowLocation(glm::ivec2 location) const;
		void changeRefreshRate(int refreshRate) const;
		void setWindowFocus(bool focus);
		void resetMousePosition() const;
		void handleMouseButtons();
		void handleKeyboard(const std::array<bool, 256>& keys);
		void handleSDL();
		void resetPrevKeys();

	private:
		void changeWindowRes(glm::ivec2 size) const;

		std::array<bool, 256> prevKeyboardKeys{};
		Components::Mouse::Buttons prevMouseKeys;
		std::array<Components::Gamepad::Buttons, 4> prevGamepadsKeys;
		std::unordered_map<int, int> controllersToComponents;
		//bool deferredPause = false;
	};
}
