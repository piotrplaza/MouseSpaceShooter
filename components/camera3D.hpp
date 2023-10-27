#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>

#include <functional>

namespace Components
{
	struct Camera3D : ComponentBase
	{
		float fov = 60.0f;
		float nearPlane = 0.1f;
		float farPlane = 100.0f;

	};
}
