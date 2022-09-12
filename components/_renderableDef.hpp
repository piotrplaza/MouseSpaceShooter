#pragma once

#include "_typeComponentMappers.hpp"

#include <tools/graphicsHelpers.hpp>

#include <commonTypes/renderLayer.hpp>

#include <ogl/shaders.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <functional>
#include <optional>

namespace Buffers
{
	struct GenericSubBuffers;
}

struct RenderableDef
{
	RenderableDef() = default;

	RenderableDef(std::vector<glm::vec3> vertices,
		std::vector<glm::vec2> texCoord = {},
		TextureComponentVariant texture = std::monostate{},
		std::optional<ComponentId> renderingSetup = std::nullopt) :
		vertices(std::move(vertices)),
		texCoord(std::move(texCoord)),
		texture(texture),
		renderingSetup(renderingSetup)
	{
	}

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texCoord;
	std::vector<glm::vec4> colors;

	std::function<glm::mat4()> modelMatrixF;
	std::function<glm::vec4()> colorF;
	TextureComponentVariant texture;
	std::optional<ComponentId> renderingSetup;

	GLenum drawMode = GL_TRIANGLES;
	GLenum bufferDataUsage = GL_STATIC_DRAW;

	bool preserveTextureRatio = false;

	bool render = true;

	struct
	{
		Buffers::GenericSubBuffers* subBuffers = nullptr;
	} loaded;

	virtual glm::mat4 getModelMatrix() const
	{
		return modelMatrixF
			? modelMatrixF()
			: glm::mat4(1.0f);
	}

	virtual std::vector<glm::vec3> getVertices(bool transformed = false) const
	{
		return transformed
			? Tools::Transform(vertices, getModelMatrix())
			: vertices;
	}

	virtual const std::vector<glm::vec4>& getColors() const
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

	virtual glm::vec2 getCenter() const
	{
		return getModelMatrix() * glm::vec4(0.0f);
	}

	virtual void step()
	{
	}
};
