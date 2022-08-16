#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>

namespace Components
{
	struct Mouse : ComponentBase
	{
		struct Buttons
		{
			bool lmb{ false };
			bool rmb{ false };
			bool mmb{ false };
			bool xmb1{ false };
			bool xmb2{ false };
			int wheel = 0;
		} pressed, released, pressing;

		glm::ivec2 delta{ 0, 0 };

		glm::vec2 getWorldSpaceDelta() const
		{
			return { delta.x, -delta.y };
		}
	};
}
