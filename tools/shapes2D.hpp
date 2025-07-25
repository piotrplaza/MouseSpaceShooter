#pragma once

#include <commonTypes/fTypes.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <optional>

namespace Tools::Shapes2D
{
	std::vector<glm::vec3> CreatePositionsOfRectangle(const glm::vec2& position = { 0.0f, 0.0f }, const glm::vec2& hSize = { 0.5f, 0.5f }, float angle = 0.0f, float z = 0.0f);
	std::vector<glm::vec2> CreateTexCoordOfRectangle(const glm::vec2& leftDown = { -0.5f, -0.5f }, const glm::vec2& rightUp = { 0.5f, 0.5f });
	std::vector<glm::vec3> CreatePositionsOfLineOfRectangles(const glm::vec2& hSize, const std::vector<glm::vec2>& positionsRanges,
		const glm::vec2& scaleRange, const glm::vec2& angleRange, const glm::vec2& stepRange, float z = 0.0f);
	std::vector<glm::vec3> CreatePositionsOfFunctionalRectangles(const glm::vec2& hSize, std::function<glm::vec2(float)> positionF,
		std::function<glm::vec2(float)> scaleF, std::function<float(float)> angleF, std::function<std::optional<float>()> inputEmitter, float z = 0.0f);
	size_t AppendPositionsOfDisc(std::vector<glm::vec3>& vertices, const glm::vec2& position, float radius, int complexity,
		const glm::mat4& modelMatrix = glm::mat4(1.0f), float z = 0.0f);
	std::vector<glm::vec3> CreatePositionsOfDisc(const glm::vec2& position, float radius, int complexity,
		const glm::mat4& modelMatrix = glm::mat4(1.0f), float z = 0.0f);
	size_t AppendPositionsOfCircle(std::vector<glm::vec3>& vertices, const glm::vec2& position, float radius, int complexity,
		const glm::mat4& modelMatrix = glm::mat4(1.0f), float z = 0.0f);
	std::vector<glm::vec3> CreatePositionsOfCircle(const glm::vec2& position, float radius, int complexity,
		const glm::mat4& modelMatrix = glm::mat4(1.0f), float z = 0.0f);
	size_t AppendPositionsOfLightning(std::vector<glm::vec3>& vertices, const glm::vec2& p1, const glm::vec2& p2,
		int segmentsNum, float frayFactor = 1.0f, float z = 0.0f);
	std::vector<glm::vec3> CreatePositionsOfLightning(const glm::vec2& p1, const glm::vec2& p2,
		int segmentsNum, float frayFactor = 1.0f, float z = 0.0f);
}
