#pragma once

#include <componentBase.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Components
{
	struct Light : ComponentBase
	{
		using ComponentBase::ComponentBase;

		glm::vec2 position;
		glm::vec3 color;
	};
}
