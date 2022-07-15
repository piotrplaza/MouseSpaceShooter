#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Components
{
	struct Light : ComponentBase
	{
		glm::vec2 position;
		glm::vec3 color;
	};
}
