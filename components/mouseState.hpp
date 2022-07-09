#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>

namespace Components
{
	struct MouseState : ComponentBase
	{
		using ComponentBase::ComponentBase;

		bool lmb{ false };
		bool rmb{ false };
		bool mmb{ false };
		bool xmb1{ false };
		bool xmb2{ false };
		int wheel = 0;
		glm::ivec2 position{ 0, 0 };
		glm::ivec2 delta{ 0, 0 };
		
		glm::ivec2 getMouseDelta() const
		{
			return delta;
		}
	};
}
