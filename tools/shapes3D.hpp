#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <functional>

struct RenderableDef;

namespace Shapes3D
{
	RenderableDef& AddRectangle(RenderableDef& renderableDef, const glm::vec2& hSize = { 0.5f, 0.5f }, const std::vector<glm::vec4>& colors = {}, const glm::mat4& transform = glm::mat4(1.0f));
	RenderableDef& AddCuboid(RenderableDef& renderableDef, const glm::vec3& hSize = { 0.5f, 0.5f, 0.5f }, const std::vector<glm::vec4>& colors = {}, const glm::mat4& transform = glm::mat4(1.0f), bool inner = false);
	RenderableDef& AddSphere(RenderableDef& renderableDef, float radius, int rings, int sectors, bool texCoords = false,
		std::function<glm::vec4(glm::vec3 normal)> colorF = nullptr, const glm::mat4& transform = glm::mat4(1.0f));
}
