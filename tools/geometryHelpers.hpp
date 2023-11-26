#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <vector>

namespace Tools
{
	std::vector<glm::vec3>& InPlaceTransformMat4(std::vector<glm::vec3>& vertices, const glm::mat4& transform);
	std::vector<glm::vec3> TransformMat4(const std::vector<glm::vec3>& vertices, const glm::mat4& transform);
	std::vector<glm::vec3>& InPlaceTransformMat3(std::vector<glm::vec3>& vertices, const glm::mat3& transform);
	std::vector<glm::vec3> TransformMat3(const std::vector<glm::vec3>& vertices, const glm::mat3& transform);
	glm::vec2 OrthoVec2(const glm::vec2& p1, const glm::vec2& p2, bool invert = false);
	void VerticesDefaultRandomTranslate(std::vector<glm::vec3>& vertices, bool loop, float randFactor);
}
