#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Components
{
	struct Light3D : ComponentBase
	{
		glm::vec3 position;
		glm::vec3 color;
	};
}
