#pragma once

#include <glm/vec2.hpp>

namespace Components
{
	struct MouseState
	{
		bool lmb{ false };
		bool rmb{ false };
		bool mmb{ false };
		glm::ivec2 position{ 0, 0 };
		glm::ivec2 delta{ 0, 0 };
		
		glm::ivec2 getMouseDelta() const
		{
			return delta;
		}
	};
}
