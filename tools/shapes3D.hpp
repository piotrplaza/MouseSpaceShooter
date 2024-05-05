#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <array>
#include <vector>
#include <functional>

#include <glm/gtc/matrix_transform.hpp>

struct RenderableDef;

namespace Components {
	struct Texture;
	struct Decoration;
}

namespace Shapes3D
{
	template <typename DecorationComponents, typename CMTexture>
	std::array<Components::Decoration*, 6> CreateCuboid(DecorationComponents& decorationComponents, const std::array<CMTexture, 6>& textures, const glm::vec3& hSize = { 0.5f, 0.5f, 0.5f })
	{
		std::array<Components::Decoration*, 6> result{};

		auto newSideF = [&, i = 0]() mutable -> auto& {
			auto& decoration = *(result[i] = &decorationComponents.emplace());
			decoration.texture = textures[i++];
			return decoration;
		};

		auto texCoordF = [](const auto& defaultTexCoord, auto) { return defaultTexCoord; };

		AddRectangle(newSideF(), { hSize.x, hSize.y }, {}, texCoordF, glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, hSize.z }), 0.0f, { 0.0f, 1.0f, 0.0f }));
		AddRectangle(newSideF(), { hSize.x, hSize.y }, {}, texCoordF, glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, -hSize.z }), glm::pi<float>(), { 0.0f, 1.0f, 0.0f }));

		AddRectangle(newSideF(), { hSize.z, hSize.y }, {}, texCoordF, glm::rotate(glm::translate(glm::mat4(1.0f), { -hSize.x, 0.0f, 0.0f }), -glm::half_pi<float>(), { 0.0f, 1.0f, 0.0f }));
		AddRectangle(newSideF(), { hSize.z, hSize.y }, {}, texCoordF, glm::rotate(glm::translate(glm::mat4(1.0f), { hSize.x, 0.0f, 0.0f }), glm::half_pi<float>(), { 0.0f, 1.0f, 0.0f }));

		AddRectangle(newSideF(), { hSize.x, hSize.z }, {}, texCoordF, glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, -hSize.y, 0.0f }), glm::half_pi<float>(), { 1.0f, 0.0f, 0.0f }));
		AddRectangle(newSideF(), { hSize.x, hSize.z }, {}, texCoordF, glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, hSize.y, 0.0f }), -glm::half_pi<float>(), { 1.0f, 0.0f, 0.0f }));

		return result;
	}

	RenderableDef& AddPoint(RenderableDef& renderableDef, const glm::vec3& pos, const glm::vec4& color = glm::vec4(1.0f), const glm::mat4& transform = glm::mat4(1.0f));
	RenderableDef& AddLine(RenderableDef& renderableDef, const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color1 = glm::vec4(1.0f), const glm::vec4& color2 = glm::vec4(1.0f), const glm::mat4& transform = glm::mat4(1.0f));
	RenderableDef& AddRectangle(RenderableDef& renderableDef, const glm::vec2& hSize = { 0.5f, 0.5f }, const std::vector<glm::vec4>& colors = {}, std::function<glm::vec2(glm::vec2, glm::vec3)> defaultAndPosToTexCoordF = nullptr,
		const glm::mat4& transform = glm::mat4(1.0f));
	RenderableDef& AddCuboid(RenderableDef& renderableDef, const glm::vec3& hSize = { 0.5f, 0.5f, 0.5f }, const std::vector<glm::vec4>& colors = {}, std::function<glm::vec2(glm::vec2, glm::vec3)> defaultAndPosToTexCoordF = nullptr,
		const glm::mat4& transform = glm::mat4(1.0f), bool inner = false);
	RenderableDef& AddSphere(RenderableDef& renderableDef, float radius = 1.0f, int rings = 20, int sectors = 20, std::function<glm::vec4(glm::vec3 normal)> colorF = nullptr, bool texCoords = false, const glm::mat4& transform = glm::mat4(1.0f));
	RenderableDef& AddCross(RenderableDef& renderableDef, glm::vec3 columnHSize = { 0.1f, 0.5f, 0.1f }, glm::vec3 rowHSize = { 0.35f, 0.1f, 0.1f }, float rowYPos = 0.15f, std::function<glm::vec2(glm::vec2, glm::vec3)> defaultAndPosToTexCoordF = nullptr,
		const glm::mat4& transform = glm::mat4(1.0f));
	RenderableDef& AddGrid(RenderableDef& renderableDef, glm::vec2 hSize, glm::ivec2 sectors, const glm::mat4& transform = glm::mat4(1.0f));
}
