#pragma once

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
		void frameSetup() const;
		void renderSetup() const;
		void frameTeardown() const;
		void changeWindowSize(glm::ivec2 size) const;
		void changeWindowLocation(glm::ivec2 location) const;
		void resetMousePosition() const;
		void handleMousePosition() const;
		void handleKeyboard(const std::array<bool, 256>& keys);

	private:
		std::array<bool, 256> prevKeys;
	};
}
