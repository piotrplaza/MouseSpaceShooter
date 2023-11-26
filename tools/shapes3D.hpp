#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <vector>

struct RenderableDef;

namespace Shapes3D
{
	void AddVerticesOfRectangle(std::vector<glm::vec3>& vertices, const glm::vec2& hSize = { 0.5f, 0.5f }, const glm::mat4& transform = glm::mat4(1.0f));
	void AddColorsOfRectangle(std::vector<glm::vec4>& colors, const std::vector<glm::vec4>& colorsOfVertices);
	void AddNormalsOfRectangle(std::vector<glm::vec3>& normals, const glm::mat3& transform = glm::mat3(1.0f));
	void AddIndicesOfRectangle(std::vector<unsigned>& indices, unsigned offset = 0);

	RenderableDef& SetRectangle(RenderableDef& renderableDef, const glm::vec2& hSize = { 0.5f, 0.5f }, const std::vector<glm::vec4>& colors = {}, const glm::mat4& transform = glm::mat4(1.0f));
	RenderableDef& SetCuboid(RenderableDef& renderableDef, const glm::vec3& hSize = { 0.5f, 0.5f, 0.5f }, const std::vector<glm::vec4>& colors = {}, const glm::mat4& transform = glm::mat4(1.0f));
}
