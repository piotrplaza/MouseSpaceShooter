#pragma once

#include <vector>
#include <functional>
#include <optional>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace Tools
{
	std::vector<glm::vec3> CreatePositionsOfRectangle(const glm::vec2& position, const glm::vec2& hSize, float angle = 0.0f, float z = 0.0f);
	std::vector<glm::vec2> CreateTexCoordOfRectangle();
	std::vector<glm::vec3> CreatePositionsOfLineOfRectangles(const glm::vec2& hSize, const std::pair<glm::vec2, glm::vec2>& positionsRange,
		const glm::vec2& scaleRange, const glm::vec2& angleRange, const glm::vec2& stepRange, float z = 0.0f);
	std::vector<glm::vec3> CreatePositionsOfFunctionalRectangles(const glm::vec2& hSize, std::function<glm::vec2(float)> positionF,
		std::function<glm::vec2(float)> scaleF, std::function<float(float)> angleF, std::function<std::optional<float>()> inputEmitter, float z = 0.0f);

	void AppendPositionsOfCircle(std::vector<glm::vec3>& result, const glm::vec2& position, float radius, int complexity,
		const glm::mat4& modelMatrix = glm::mat4(1.0f), float z = 0.0f);

	std::vector<glm::vec3> CreatePositionsOfCircle(const glm::vec2& position, float radius, int complexity,
		const glm::mat4& modelMatrix = glm::mat4(1.0f), float z = 0.0f);

	std::vector<glm::vec3> CreatePositionsOfLightning(const glm::vec2& p1, const glm::vec2& p2,
		int segmentsNum, float frayFactor = 1.0f, float z = 0.0f);

	std::vector<glm::vec3> Transform(const std::vector<glm::vec3>& positions, const glm::mat4& transformation);
}
