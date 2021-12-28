#pragma once

#include <glm/vec2.hpp>

#include <componentBase.hpp>

namespace Components
{
	struct ScreenInfo : ComponentBase
	{
		using ComponentBase::ComponentBase;

		glm::ivec2 windowSize{ 0, 0 };
		glm::ivec2 windowLocation{ 0, 0 };
		glm::ivec2 windowCenterInScreenSpace{ 0, 0 };
	};
}
