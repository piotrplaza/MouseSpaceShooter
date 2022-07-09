#pragma once

#include "_componentBase.hpp"

#include <glm/vec4.hpp>

namespace Components
{
	struct GraphicsSettings : ComponentBase
	{
		using ComponentBase::ComponentBase;

		glm::vec4 clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
		glm::vec4 defaultColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	};
}
