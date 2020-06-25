#pragma once

#include <functional>

#include <glm/vec2.hpp>

namespace Components
{
	struct Camera
	{
		std::function<glm::vec2()> mainActorPositionF;
		std::function<float()> projectionHSizeF;

		float positionTransitionFactor = 1.0f;
		float projectionTransitionFactor = 1.0f;
	};
}
