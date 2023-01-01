#pragma once

#include <commonTypes/componentId.hpp>
#include <glm/vec2.hpp>

namespace Levels
{
	struct ControlPoint
	{
		ComponentId decorationId;
		glm::vec2 pos;
	};
}
