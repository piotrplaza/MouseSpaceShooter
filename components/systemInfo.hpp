#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>

namespace Components
{
	struct SystemInfo : ComponentBase
	{
		struct Screen
		{
			glm::ivec2 framebufferRes{ 0, 0 };
			glm::ivec2 windowSize{ 0, 0 };
			glm::ivec2 windowLocation{ 0, 0 };
			glm::ivec2 windowCenterInScreenSpace{ 0, 0 };
			int refreshRate{ 0 };

			float getAspectRatio() const
			{
				assert(framebufferRes.y != 0);
				return (float)framebufferRes.x / framebufferRes.y;
			}

			float getRefreshDuration() const
			{
				assert(refreshRate != 0);
				return 1.0f / refreshRate;
			}

			glm::vec2 getNormalizedFramebufferRes() const
			{
				const float shortestSide = (float)std::min(framebufferRes.x, framebufferRes.y);
				assert(shortestSide != 0);
				return glm::vec2(framebufferRes) / shortestSide;
			}
		} screen;

		struct Limits
		{
			int maxTextureUnits{ 0 };
		} limits;
	};
}
