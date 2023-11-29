#include "shapes3D.hpp"

#include "geometryHelpers.hpp"

#include <components/_renderableDef.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace
{
	void AddVerticesOfRectangle(std::vector<glm::vec3>& vertices, const glm::vec2& hSize, const glm::mat4& transform)
	{
		std::vector<glm::vec3> rectangleVertices = {
			{-hSize.x, -hSize.y, 0.0f},
			{hSize.x, -hSize.y, 0.0f},
			{-hSize.x, hSize.y, 0.0f},
			{hSize.x, hSize.y, 0.0f}
		};
		Tools::InPlaceTransformMat4(rectangleVertices, transform);
		vertices.insert(vertices.end(), rectangleVertices.begin(), rectangleVertices.end());
	}

	void AddColorsOfRectangle(std::vector<glm::vec4>& colors, const std::vector<glm::vec4>& colorsOfVertices)
	{
		if (colorsOfVertices.empty())
			return;

		if (colorsOfVertices.size() >= 4)
			colors.insert(colors.end(), colorsOfVertices.begin(), std::next(colorsOfVertices.begin(), 4));
		else
		{
			std::vector<glm::vec4> repeatedColors;
			repeatedColors.reserve(4);
			for (unsigned i = 0; i < 4; ++i)
				repeatedColors.push_back(colorsOfVertices[i % colorsOfVertices.size()]);
			colors.insert(colors.end(), repeatedColors.begin(), repeatedColors.end());
		}
	}

	void AddNormalsOfRectangle(std::vector<glm::vec3>& normals, const glm::mat3& transform)
	{
		std::vector<glm::vec3> rectangleNormals = {
			{0.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 1.0f}
		};
		Tools::InPlaceTransformMat3(rectangleNormals, transform);
		normals.insert(normals.end(), rectangleNormals.begin(), rectangleNormals.end());
	}

	void AddIndicesOfRectangle(std::vector<unsigned>& indices, unsigned offset)
	{
		std::vector<unsigned> rectangleIndices = {
			0, 1, 2, 2, 1, 3
		};
		for (auto& index : rectangleIndices)
			index += offset;
		indices.insert(indices.end(), rectangleIndices.begin(), rectangleIndices.end());
	}
}

namespace Shapes3D
{
	RenderableDef& AddRectangle(RenderableDef& renderableDef, const glm::vec2& hSize, const std::vector<glm::vec4>& colors, const glm::mat4& transform)
	{
		unsigned offset = renderableDef.vertices.size();

		if (!renderableDef.params3D)
			renderableDef.params3D = RenderableDef::Params3D{};

		AddVerticesOfRectangle(renderableDef.vertices, hSize, transform);
		AddColorsOfRectangle(renderableDef.colors, colors);
		AddNormalsOfRectangle(renderableDef.params3D->normals_, transform);
		AddIndicesOfRectangle(renderableDef.indices, offset);

		return renderableDef;
	}

	RenderableDef& AddCuboid(RenderableDef& renderableDef, const glm::vec3& hSize, const std::vector<glm::vec4>& colors, const glm::mat4& transform)
	{
		auto getColorsOfVertices = [&, col = std::vector<glm::vec4>{}, j = 0]() mutable -> const std::vector<glm::vec4>& {
			if (colors.empty())
				return col;

			col.reserve(4);
			for (unsigned i = 0; i < 4; ++i)
			{
				col.push_back(colors[j++ % colors.size()]);
			}
			return col;
		};

		AddRectangle(renderableDef, { hSize.x, hSize.y }, getColorsOfVertices(), transform * glm::translate(glm::mat4(1.0f), {0.0f, 0.0f, hSize.z}));
		AddRectangle(renderableDef, { hSize.x, hSize.y }, getColorsOfVertices(), transform * glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, -hSize.z }), glm::pi<float>(), {0.0f, 1.0f, 0.0f}));

		AddRectangle(renderableDef, { hSize.z, hSize.y }, getColorsOfVertices(), transform * glm::rotate(glm::translate(glm::mat4(1.0f), { -hSize.x, 0.0f, 0.0f }), -glm::half_pi<float>(), { 0.0f, 1.0f, 0.0f }));
		AddRectangle(renderableDef, { hSize.z, hSize.y }, getColorsOfVertices(), transform * glm::rotate(glm::translate(glm::mat4(1.0f), { hSize.x, 0.0f, 0.0f }), glm::half_pi<float>(), { 0.0f, 1.0f, 0.0f }));

		AddRectangle(renderableDef, { hSize.x, hSize.z }, getColorsOfVertices(), transform * glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, -hSize.y, 0.0f }), glm::half_pi<float>(), { 1.0f, 0.0f, 0.0f }));
		AddRectangle(renderableDef, { hSize.x, hSize.z }, getColorsOfVertices(), transform * glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, hSize.y, 0.0f }), -glm::half_pi<float>(), { 1.0f, 0.0f, 0.0f }));

		return renderableDef;
	}
}
