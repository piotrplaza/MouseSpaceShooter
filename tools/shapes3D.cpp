#include "shapes3D.hpp"

#include "geometryHelpers.hpp"

#include <components/_renderableDef.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

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

	RenderableDef& AddCuboid(RenderableDef& renderableDef, const glm::vec3& hSize, const std::vector<glm::vec4>& colors, const glm::mat4& transform, bool inner)
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

		const float innerRotation = inner * glm::pi<float>();

		AddRectangle(renderableDef, { hSize.x, hSize.y }, getColorsOfVertices(), transform * glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, hSize.z }), innerRotation, { 0.0f, 1.0f, 0.0f }));
		AddRectangle(renderableDef, { hSize.x, hSize.y }, getColorsOfVertices(), transform * glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, -hSize.z }), glm::pi<float>() + innerRotation, {0.0f, 1.0f, 0.0f}));

		AddRectangle(renderableDef, { hSize.z, hSize.y }, getColorsOfVertices(), transform * glm::rotate(glm::translate(glm::mat4(1.0f), { -hSize.x, 0.0f, 0.0f }), -glm::half_pi<float>() + innerRotation, { 0.0f, 1.0f, 0.0f }));
		AddRectangle(renderableDef, { hSize.z, hSize.y }, getColorsOfVertices(), transform * glm::rotate(glm::translate(glm::mat4(1.0f), { hSize.x, 0.0f, 0.0f }), glm::half_pi<float>() + innerRotation, { 0.0f, 1.0f, 0.0f }));

		AddRectangle(renderableDef, { hSize.x, hSize.z }, getColorsOfVertices(), transform * glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, -hSize.y, 0.0f }), glm::half_pi<float>() + innerRotation, { 1.0f, 0.0f, 0.0f }));
		AddRectangle(renderableDef, { hSize.x, hSize.z }, getColorsOfVertices(), transform * glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, hSize.y, 0.0f }), -glm::half_pi<float>() + innerRotation, { 1.0f, 0.0f, 0.0f }));

		return renderableDef;
	}

	RenderableDef& AddSphere(RenderableDef& renderableDef, float radius, int rings, int sectors, bool texCoords, std::function<glm::vec4(glm::vec3 normal)> colorF, const glm::mat4& transform)
	{
		const float R = 1.0f / static_cast<float>(++rings - 1);
		const float S = 1.0f / static_cast<float>(++sectors - 1);

		if (!renderableDef.params3D)
			renderableDef.params3D = RenderableDef::Params3D{};

		const size_t offset = renderableDef.vertices.size();
		renderableDef.vertices.reserve(offset + rings * sectors);
		renderableDef.colors.reserve(offset + rings * sectors);
		renderableDef.params3D->normals_.reserve(offset + rings * sectors);

		if (texCoords)
			renderableDef.texCoord.reserve(offset + rings * sectors);

		const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(transform));

		for (int r = 0; r < rings; ++r)
		{
			for (int s = 0; s < sectors; ++s)
			{
				const glm::vec3 normal(
					sin(-glm::half_pi<float>() + glm::pi<float>() * r * R),
					cos(2 * glm::pi<float>() * s * S) * sin(glm::pi<float>() * r * R),
					sin(2 * glm::pi<float>() * s * S) * sin(glm::pi<float>() * r * R));

				const glm::vec3 vertex = normal * radius;

				if (texCoords)
					renderableDef.texCoord.push_back(glm::vec2(s * S, r * R));

				renderableDef.vertices.push_back(glm::vec3(transform * glm::vec4(vertex, 1.0f)));

				const glm::vec3 finalNormal = glm::normalize(normalMatrix * normal);

				if (colorF)
					renderableDef.colors.push_back(colorF(finalNormal));
				else
					renderableDef.colors.emplace_back(1.0f);

				renderableDef.params3D->normals_.push_back(finalNormal);
			}
		}

		renderableDef.indices.reserve(renderableDef.indices.size() + 6 * (rings - 1) * (sectors - 1));
		for (int r = 0; r < rings - 1; ++r)
		{
			for (int s = 0; s < sectors - 1; ++s)
			{
				renderableDef.indices.push_back(offset + r * sectors + s);
				renderableDef.indices.push_back(offset + r * sectors + (s + 1));
				renderableDef.indices.push_back(offset + (r + 1) * sectors + (s + 1));

				renderableDef.indices.push_back(offset + r * sectors + s);
				renderableDef.indices.push_back(offset + (r + 1) * sectors + (s + 1));
				renderableDef.indices.push_back(offset + (r + 1) * sectors + s);
			}
		}

		return renderableDef;
	}
}
