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
		struct EulerRotation: glm::vec3
		{
			using glm::vec3::vec3;
		};

		struct LookAtRotation
		{
			LookAtRotation(const glm::vec3& target = glm::vec3(0.0f), const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f)):
				target(target),
				up(up)
			{
			}

			glm::vec3 target;
			glm::vec3 up;
		};

		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		std::variant<EulerRotation, LookAtRotation> rotation = EulerRotation(0.0f, 0.0f, 0.0f);

		float fov = 60.0f / 360.0f * glm::two_pi<float>();
		float nearPlane = 0.1f;
		float farPlane = 1000.0f;
	};
}
