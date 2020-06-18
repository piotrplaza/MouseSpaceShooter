#pragma once

#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace tools
{
	std::vector<glm::vec3> createCircleVertices(glm::vec2 position, float radius, int complexity, const glm::mat4& modelMatrix = glm::mat4(1.0f));
}
