#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>

#include <functional>

namespace Components
{
	struct Camera : ComponentBase
	{
		std::function<glm::vec2()> targetPositionF = []() { return glm::vec2(0.0f, 0.0f); };
		std::function<float()> targetProjectionHSizeF = []() { return 10.0f; };

		float positionTransitionFactor = 1.0f;
		float projectionTransitionFactor = 1.0f;

		struct
		{
			glm::vec2 position{ 0.0f };
			glm::vec2 prevPosition{ 0.0f };

			float projectionHSize = 0.0f;
			float prevProjectionHSize = 0.0f;
		} details;
	};
}
