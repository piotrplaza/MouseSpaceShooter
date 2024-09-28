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
			assert(windowSize.y != 0);
			return (float)windowSize.x / windowSize.y;
		}

		float getRefreshDuration() const
		{
			assert(refreshRate != 0);
			return 1.0f / refreshRate;
		}

		glm::vec2 getNormalizedWindowSize() const
		{
			const float shortestSide = (float)std::min(windowSize.x, windowSize.y);
			assert(shortestSide != 0);
			return glm::vec2(windowSize) / shortestSide;
		}
	};
}
