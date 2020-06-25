#pragma once

#include <functional>

#include <glm/vec2.hpp>

namespace Components
{
	struct Camera
	{
		std::function<glm::vec2()> mainActorPositionF;
		std::function<float()> verticalProjectionHSizeF;
	};
}
