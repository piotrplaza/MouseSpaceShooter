#pragma once

#include "_typeComponentMappers.hpp"
#include "_decorationDef.hpp"

#include <tools/graphicsHelpers.hpp>

#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/renderLayer.hpp>

#include <ogl/shaders.hpp>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <vector>
#include <optional>

namespace Buffers
{
	struct GenericBuffers;
}

struct Renderable
{
	Renderable(TextureComponentVariant texture = std::monostate{},
		std::optional<ComponentId> renderingSetup = std::nullopt,
		RenderLayer renderLayer = RenderLayer::Midground,
		std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt,
		std::vector<glm::vec3> vertices = {},
		std::vector<glm::vec4> colors = {},
		std::vector<glm::vec2> texCoord = {}):
		texture(texture),
		renderingSetup(renderingSetup),
		renderLayer(renderLayer),
		customShadersProgram(customShadersProgram),
		vertices(std::move(vertices)),
		colors(std::move(colors)),
		texCoord(std::move(texCoord))
	{
	}

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec4> colors;
	std::vector<glm::vec2> texCoord;

	TextureComponentVariant texture;
	std::optional<ComponentId> renderingSetup;
	std::optional<Shaders::ProgramId> customShadersProgram;

	ResolutionMode resolutionMode = ResolutionMode::Normal;
	RenderLayer renderLayer = RenderLayer::Midground;

	GLenum drawMode = GL_TRIANGLES;
	GLenum bufferDataUsage = GL_STATIC_DRAW;

	bool preserveTextureRatio = false;

	bool render = true;

	std::vector<DecorationDef> subsequence;
	unsigned posInSubsequence = 0;

	struct
	{
		Buffers::GenericBuffers* buffers = nullptr;
	} loaded;

	virtual std::vector<glm::vec3> getVertices(bool transformed = false) const
	{
		return transformed
			? Tools::Transform(vertices, getModelMatrix())
			: vertices;
	}

	const std::vector<glm::vec4>& getColors() const
	{
		return colors;
	}

	virtual const std::vector<glm::vec2> getTexCoord(bool transformed = false) const
	{
		const auto vertices = getVertices(transformed);
		if (texCoord.empty())
		{
			return std::vector<glm::vec2>(vertices.begin(), vertices.end());
		}
		else if (texCoord.size() < vertices.size())
		{
			std::vector<glm::vec2> cyclicTexCoord;
			cyclicTexCoord.reserve(vertices.size());
			for (size_t i = 0; i < vertices.size(); ++i)
				cyclicTexCoord.push_back(texCoord[i % texCoord.size()]);
			return cyclicTexCoord;
		}
		else
		{
			assert(texCoord.size() == vertices.size());
			return texCoord;
		}
	}

	virtual glm::mat4 getModelMatrix() const = 0;
};
