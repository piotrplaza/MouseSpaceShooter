#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Components
{
	struct Light2D : ComponentBase
	{
		glm::vec2 position{ 0.0f };
		glm::vec3 color{ 1.0f };
	};
}
