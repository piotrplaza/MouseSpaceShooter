#pragma once

#include <glm/vec4.hpp>

namespace Components
{
	struct GraphicsSettings
	{
		glm::vec4 basicLevelColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 texturedLevelColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 basicDecorationsColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 texturedDecorationsColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	};
}
