#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Components
{
	struct Light3D : ComponentBase
	{
		Light3D() = default;
		Light3D(glm::vec3 position, glm::vec3 color = glm::vec3(1.0f), float attenuation = 0.0f, bool viewSpace = false)
			: position(position)
			, color(color)
			, attenuation(attenuation)
			, viewSpace(viewSpace)
		{
		}

		glm::vec3 position{ 0.0f };
		glm::vec3 color{ 1.0f };
		float attenuation{ 0.0f };
		bool viewSpace{ false };
	};
}
