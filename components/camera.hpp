#pragma once

#include <functional>

#include <glm/vec2.hpp>

namespace Components
{
	struct Camera
	{
		std::function<glm::vec2()> targetPositionF;
		std::function<float()> targetProjectionHSizeF;

		float positionTransitionFactor = 1.0f;
		float projectionTransitionFactor = 1.0f;

		glm::vec2 prevPosition{ 0.0f };
		float prevProjectionHSize = 0.0f;
	};
}
