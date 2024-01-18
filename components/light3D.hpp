#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Components
{
	struct Light3D : ComponentBase
	{
		glm::vec3 position{ 0.0f };
		glm::vec3 color{ 1.0f };
		float attenuation{ 0.0f };
	};
}
