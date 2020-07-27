#pragma once

#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace Tools
{
	std::vector<glm::vec3> CreateRectanglePositions(const glm::vec2& position, const glm::vec2& hSize);
	std::vector<glm::vec2> CreateRectangleTexCoord();
	std::vector<glm::vec3> CreateLineOfRectanglesPositions(const glm::vec2& hSize, const std::pair<glm::vec2, glm::vec2>& positionsRange,
		const glm::vec2& scaleRange, const glm::vec2& stepRange, const glm::vec2& angleRange);

	std::vector<glm::vec3> CreateCirclePositions(const glm::vec2& position, float radius, int complexity,
		const glm::mat4& modelMatrix = glm::mat4(1.0f));

	std::vector<glm::vec3> CreateLightningPositions(const glm::vec2& p1, const glm::vec2& p2,
		int segmentsNum, float frayFactor = 1.0f, float zValue = 0.0f);

	std::vector<glm::vec3> Transform(const std::vector<glm::vec3>& positions, const glm::mat4& transformation);
}
