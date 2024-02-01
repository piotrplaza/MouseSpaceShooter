#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>

#include <variant>

namespace Components
{
	struct Camera3D : ComponentBase
	{
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		std::variant<glm::vec3, std::pair<glm::vec3, glm::vec3>> rotation = glm::vec3(0.0f, 0.0f, 0.0f);

		float fov = 60.0f / 360.0f * glm::two_pi<float>();
		float nearPlane = 0.1f;
		float farPlane = 100.0f;
	};
}
