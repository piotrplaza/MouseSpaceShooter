#pragma once

#include <components/mouseState.hpp>

#include <glm/vec2.hpp>

#include <array>

namespace Systems
{
	class StateController
	{
	public:
		StateController();
		~StateController();

		void postInit() const;
		void stepSetup() const;
		void renderSetup() const;
		void stepTeardown() const;
		void changeWindowSize(glm::ivec2 size) const;
		void changeWindowLocation(glm::ivec2 location) const;
		void resetMousePosition() const;
		void handleMouseButtons();
		void handleKeyboard(const std::array<bool, 256>& keys);

	private:
		std::array<bool, 256> prevKeyboardKeys{};
		Components::MouseState::Buttons prevMouseKeys;
	};
}
