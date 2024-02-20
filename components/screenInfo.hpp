#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>

namespace Components
{
	struct ScreenInfo : ComponentBase
	{
		glm::ivec2 windowSize{ 0, 0 };
		glm::ivec2 windowLocation{ 0, 0 };
		glm::ivec2 windowCenterInScreenSpace{ 0, 0 };
		int refreshRate{ 0 };

		float getAspectRatio() const
		{
			return (float)windowSize.x / windowSize.y;
		}

		float getRefreshDuration() const
		{
			return 1.0f / refreshRate;
		}
	};
}
