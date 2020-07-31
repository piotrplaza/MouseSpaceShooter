#pragma once

#include <glm/vec4.hpp>

namespace Components
{
	struct GraphicsSettings
	{
		glm::vec4 clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
		glm::vec4 defaultColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	};
}
