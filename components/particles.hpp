#pragma once

#include "_componentBase.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>

namespace Components
{
	struct Particles : ComponentBase
	{
		Particles() = default;
		Particles(std::vector<glm::vec3> positions, std::vector<glm::vec4> colors) :
			positions(positions),
			colors(colors)
		{
		}

		std::vector<glm::vec3> positions;
		std::vector<glm::vec4> colors;
		glm::vec3 center{ 0.0f };
	};
}
