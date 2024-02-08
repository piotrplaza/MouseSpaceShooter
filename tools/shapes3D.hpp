#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <functional>

struct RenderableDef;

namespace Shapes3D
{
	RenderableDef& AddPoint(RenderableDef& renderableDef, const glm::vec3& pos, const glm::vec4& color = glm::vec4(1.0f), const glm::mat4& transform = glm::mat4(1.0f));
	RenderableDef& AddLine(RenderableDef& renderableDef, const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color1 = glm::vec4(1.0f), const glm::vec4& color2 = glm::vec4(1.0f), const glm::mat4& transform = glm::mat4(1.0f));
	RenderableDef& AddRectangle(RenderableDef& renderableDef, const glm::vec2& hSize = { 0.5f, 0.5f }, const std::vector<glm::vec4>& colors = {}, std::function<glm::vec2(glm::vec2, glm::vec3)> defaultAndPosToTexCoordF = nullptr, const glm::mat4& transform = glm::mat4(1.0f));
	RenderableDef& AddCuboid(RenderableDef& renderableDef, const glm::vec3& hSize = { 0.5f, 0.5f, 0.5f }, const std::vector<glm::vec4>& colors = {}, std::function<glm::vec2(glm::vec2, glm::vec3)> defaultAndPosToTexCoordF = nullptr,
		const glm::mat4& transform = glm::mat4(1.0f), bool inner = false);
	RenderableDef& AddSphere(RenderableDef& renderableDef, float radius, int rings, int sectors, std::function<glm::vec4(glm::vec3 normal)> colorF = nullptr, bool texCoords = false, const glm::mat4& transform = glm::mat4(1.0f));
	RenderableDef& AddCross(RenderableDef& renderableDef, glm::vec3 columnHSize = { 0.1f, 0.5f, 0.1f }, glm::vec3 rowHSize = { 0.35f, 0.1f, 0.1f }, float rowYPos = 0.15f, std::function<glm::vec2(glm::vec2, glm::vec3)> defaultAndPosToTexCoordF = nullptr,
		const glm::mat4& transform = glm::mat4(1.0f));
	RenderableDef& AddGrid(RenderableDef& renderableDef, glm::vec2 hSize, glm::ivec2 sectors, const glm::mat4& transform = glm::mat4(1.0f));
}
