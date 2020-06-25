#pragma once

#include <glm/vec2.hpp>

namespace Components
{
	struct ScreenInfo
	{
		glm::ivec2 windowSize{ 0, 0 };
		glm::ivec2 windowLocation{ 0, 0 };
		glm::ivec2 windowCenterInScreenSpace{ 0, 0 };
	};
}
