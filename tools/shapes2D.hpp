#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <functional>
#include <optional>

namespace Shapes2D
{
	std::vector<glm::vec3> CreateVerticesOfRectangle(const glm::vec2& position = { 0.0f, 0.0f }, const glm::vec2& hSize = { 0.5f, 0.5f }, float angle = 0.0f, float z = 0.0f);
	std::vector<glm::vec2> CreateTexCoordOfRectangle(const glm::vec2& leftDown = { -0.5f, -0.5f }, const glm::vec2& bottomUp = { 0.5f, 0.5f });
	std::vector<glm::vec3> CreateVerticesOfLineOfRectangles(const glm::vec2& hSize, const std::vector<glm::vec2>& positionsRanges,
		const glm::vec2& scaleRange, const glm::vec2& angleRange, const glm::vec2& stepRange, float z = 0.0f);
	std::vector<glm::vec3> CreateVerticesOfFunctionalRectangles(const glm::vec2& hSize, std::function<glm::vec2(float)> positionF,
		std::function<glm::vec2(float)> scaleF, std::function<float(float)> angleF, std::function<std::optional<float>()> inputEmitter, float z = 0.0f);
	void AppendVerticesOfCircle(std::vector<glm::vec3>& result, const glm::vec2& position, float radius, int complexity,
		const glm::mat4& modelMatrix = glm::mat4(1.0f), float z = 0.0f);
	std::vector<glm::vec3> CreateVerticesOfCircle(const glm::vec2& position, float radius, int complexity,
		const glm::mat4& modelMatrix = glm::mat4(1.0f), float z = 0.0f);
	std::vector<glm::vec3> CreateVerticesOfLightning(const glm::vec2& p1, const glm::vec2& p2,
		int segmentsNum, float frayFactor = 1.0f, float z = 0.0f);
}
