#pragma once

#include <glm/vec2.hpp>

namespace Systems
{
	class StateController
	{
	public:
		StateController();
		~StateController();

		void postInit() const;
		void frameSetup() const;
		void frameTeardown() const;
		void changeWindowSize(glm::ivec2 size) const;
		void changeWindowLocation(glm::ivec2 location) const;
		void resetMousePosition() const;
		void handleMousePosition() const;
		void handleKeyboard(bool const* const keys) const;
	};
}
