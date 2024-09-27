#include "Shapes3D.hpp"

#include "geometryHelpers.hpp"

#include <components/_renderableDef.hpp>

#include <glm/gtc/matrix_inverse.hpp>

namespace
{
	void AddVerticesAndTexCoordsOfRectangle(std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& texCoords, const glm::vec2& hSize, std::function<glm::vec2(glm::vec2, glm::vec3)> defaultAndPosToTexCoordF, const glm::mat4& transform)
	{
		std::vector<glm::vec3> rectangleVertices = {
			{-hSize.x, -hSize.y, 0.0f},
			{hSize.x, -hSize.y, 0.0f},
			{-hSize.x, hSize.y, 0.0f},
			{hSize.x, hSize.y, 0.0f}
		};

		static const std::vector<glm::vec2> defaultRectangleTexCoords = {
			{-0.5f, -0.5f},
			{0.5f, -0.5f},
			{-0.5f, 0.5f},
			{0.5f, 0.5f}
		};

		Tools::InPlaceTransformMat4(rectangleVertices, transform);
		vertices.insert(vertices.end(), rectangleVertices.begin(), rectangleVertices.end());

		if (defaultAndPosToTexCoordF)
		{
			//texCoords.reserve(texCoords.size() + rectangleVertices.size()); // Extremely slowing down if multiple calls.
			for (size_t i = 0; i < rectangleVertices.size(); ++i)
				texCoords.push_back(defaultAndPosToTexCoordF(defaultRectangleTexCoords[i], rectangleVertices[i]));
		}
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

namespace Tools::Shapes3D
{
	RenderableDef& AddPoint(RenderableDef& renderableDef, const glm::vec3& pos, const glm::vec4& color, const glm::mat4& transform)
	{
		if (!renderableDef.params3D)
			renderableDef.params3D = RenderableDef::Params3D{}.ambient(1.0f).diffuse(0.0f).specular(0.0f);

		renderableDef.vertices.push_back(glm::vec3(transform * glm::vec4(pos, 1.0f)));
		renderableDef.colors.push_back(color);
		renderableDef.params3D->addNormals({ {0.0f, 0.0f, 0.0f} });
		renderableDef.drawMode = GL_POINTS;

		return renderableDef;
	}

	RenderableDef& AddLine(RenderableDef& renderableDef, const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color1, const glm::vec4& color2, const glm::mat4& transform)
	{
		if (!renderableDef.params3D)
			renderableDef.params3D = RenderableDef::Params3D{}.ambient(1.0f).diffuse(0.0f).specular(0.0f);

		renderableDef.vertices.push_back(glm::vec3(transform * glm::vec4(p1, 1.0f)));
		renderableDef.vertices.push_back(glm::vec3(transform * glm::vec4(p2, 1.0f)));
		renderableDef.colors.push_back(color1);
		renderableDef.colors.push_back(color2);
		renderableDef.params3D->addNormals({ {0.0f, 0.0f, 0.0f} });
		renderableDef.params3D->addNormals({ {0.0f, 0.0f, 0.0f} });
		renderableDef.drawMode = GL_LINES;

		return renderableDef;
	}

	RenderableDef& AddRectangle(RenderableDef& renderableDef, const glm::vec2& hSize, const std::vector<glm::vec4>& colors, std::function<glm::vec2(glm::vec2, glm::vec3)> defaultAndPosToTexCoordF, const glm::mat4& transform)
	{
		unsigned offset = renderableDef.vertices.size();

		if (!renderableDef.params3D)
			renderableDef.params3D = RenderableDef::Params3D{};

		AddVerticesAndTexCoordsOfRectangle(renderableDef.vertices, renderableDef.texCoord, hSize, defaultAndPosToTexCoordF, transform);
		AddColorsOfRectangle(renderableDef.colors, colors);
		AddNormalsOfRectangle(renderableDef.params3D->normals_, transform);

		AddIndicesOfRectangle(renderableDef.indices, offset);

		return renderableDef;
	}

	RenderableDef& AddCuboid(RenderableDef& renderableDef, const glm::vec3& hSize, const std::vector<glm::vec4>& colors, std::function<glm::vec2(glm::vec2, glm::vec3)> defaultAndPosToTexCoordF, const glm::mat4& transform, bool inner)
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

		AddRectangle(renderableDef, { hSize.x, hSize.y }, getColorsOfVertices(), defaultAndPosToTexCoordF, transform * glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, hSize.z }), innerRotation, { 0.0f, 1.0f, 0.0f }));
		AddRectangle(renderableDef, { hSize.x, hSize.y }, getColorsOfVertices(), defaultAndPosToTexCoordF, transform * glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, -hSize.z }), glm::pi<float>() + innerRotation, {0.0f, 1.0f, 0.0f}));

		AddRectangle(renderableDef, { hSize.z, hSize.y }, getColorsOfVertices(), defaultAndPosToTexCoordF, transform * glm::rotate(glm::translate(glm::mat4(1.0f), { -hSize.x, 0.0f, 0.0f }), -glm::half_pi<float>() + innerRotation, { 0.0f, 1.0f, 0.0f }));
		AddRectangle(renderableDef, { hSize.z, hSize.y }, getColorsOfVertices(), defaultAndPosToTexCoordF, transform * glm::rotate(glm::translate(glm::mat4(1.0f), { hSize.x, 0.0f, 0.0f }), glm::half_pi<float>() + innerRotation, { 0.0f, 1.0f, 0.0f }));

		AddRectangle(renderableDef, { hSize.x, hSize.z }, getColorsOfVertices(), defaultAndPosToTexCoordF, transform * glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, -hSize.y, 0.0f }), glm::half_pi<float>() + innerRotation, { 1.0f, 0.0f, 0.0f }));
		AddRectangle(renderableDef, { hSize.x, hSize.z }, getColorsOfVertices(), defaultAndPosToTexCoordF, transform * glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, hSize.y, 0.0f }), -glm::half_pi<float>() + innerRotation, { 1.0f, 0.0f, 0.0f }));

		return renderableDef;
	}

	RenderableDef& AddSphere(RenderableDef& renderableDef, float radius, int rings, int sectors, std::function<glm::vec4(glm::vec3 normal)> colorF, bool texCoords, const glm::mat4& transform)
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

	RenderableDef& AddCross(RenderableDef& renderableDef, glm::vec3 columnHSize, glm::vec3 rowHSize, float rowYPos, std::function<glm::vec2(glm::vec2, glm::vec3)> defaultAndPosToTexCoordF, const glm::mat4& transform)
	{
		AddCuboid(renderableDef, columnHSize, {}, defaultAndPosToTexCoordF, transform * glm::translate(glm::mat4(1.0f), { 0.0f, columnHSize.y, 0.0f }));
		AddCuboid(renderableDef, rowHSize, {}, defaultAndPosToTexCoordF, transform * glm::translate(glm::mat4(1.0f), { 0.0f, columnHSize.y + rowYPos, 0.0f }));
		return renderableDef;
	}

	RenderableDef& AddGrid(RenderableDef& renderableDef, glm::vec2 hSize, glm::ivec2 sectors, const glm::mat4& transform)
	{
		const glm::vec2 step = hSize / glm::vec2(sectors);
		const glm::vec2 offset = -hSize / 2.0f;

		for (int i = 0; i <= sectors.x; ++i)
		{
			const float x = offset.x + step.x * i;
			AddLine(renderableDef, { x, 0.0f, -hSize.y / 2.0f }, { x, 0.0f, hSize.y / 2.0f }, glm::vec4(1.0f), glm::vec4(1.0f), transform);
		}

		for (int i = 0; i <= sectors.y; ++i)
		{
			const float z = offset.y + step.y * i;
			AddLine(renderableDef, { -hSize.x / 2.0f, 0.0f, z }, { hSize.x / 2.0f, 0.0f, z }, glm::vec4(1.0f), glm::vec4(1.0f), transform);
		}

		return renderableDef;
	}

	RenderableDef& AddWiredCuboid(RenderableDef& renderableDef, const glm::vec3& hSize, const glm::vec4& color, const glm::mat4& transform )
	{
		AddLine(renderableDef, { -hSize.x, -hSize.y, -hSize.z }, { hSize.x, -hSize.y, -hSize.z }, color, color, transform);
		AddLine(renderableDef, { hSize.x, -hSize.y, -hSize.z }, { hSize.x, hSize.y, -hSize.z }, color, color, transform);
		AddLine(renderableDef, { hSize.x, hSize.y, -hSize.z }, { -hSize.x, hSize.y, -hSize.z }, color, color, transform);
		AddLine(renderableDef, { -hSize.x, hSize.y, -hSize.z }, { -hSize.x, -hSize.y, -hSize.z }, color, color, transform);

		AddLine(renderableDef, { -hSize.x, -hSize.y, hSize.z }, { hSize.x, -hSize.y, hSize.z }, color, color, transform);
		AddLine(renderableDef, { hSize.x, -hSize.y, hSize.z }, { hSize.x, hSize.y, hSize.z }, color, color, transform);
		AddLine(renderableDef, { hSize.x, hSize.y, hSize.z }, { -hSize.x, hSize.y, hSize.z }, color, color, transform);
		AddLine(renderableDef, { -hSize.x, hSize.y, hSize.z }, { -hSize.x, -hSize.y, hSize.z }, color, color, transform);

		AddLine(renderableDef, { -hSize.x, -hSize.y, -hSize.z }, { -hSize.x, -hSize.y, hSize.z }, color, color, transform);
		AddLine(renderableDef, { hSize.x, -hSize.y, -hSize.z }, { hSize.x, -hSize.y, hSize.z }, color, color, transform);
		AddLine(renderableDef, { hSize.x, hSize.y, -hSize.z }, { hSize.x, hSize.y, hSize.z }, color, color, transform);
		AddLine(renderableDef, { -hSize.x, hSize.y, -hSize.z }, { -hSize.x, hSize.y, hSize.z }, color, color, transform);

		return renderableDef;
	}
}
