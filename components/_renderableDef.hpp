#pragma once

#include <tools/geometryHelpers.hpp>

#include <commonTypes/typeComponentMappers.hpp>
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
	struct Params3D
	{
		Params3D& normals(std::vector<glm::vec3> value)
		{
			normals_ = std::move(value);
			return *this;
		}

		Params3D& ambientF(std::function<float()> value)
		{
			ambientF_ = std::move(value);
			return *this;
		}

		Params3D& diffuseF(std::function<float()> value)
		{
			diffuseF_ = std::move(value);
			return *this;
		}

		Params3D& specularF(std::function<float()> value)
		{
			specularF_ = std::move(value);
			return *this;
		}

		Params3D& specularFocusF(std::function<float()> value)
		{
			specularFocusF_ = std::move(value);
			return *this;
		}

		std::vector<glm::vec3> normals_;
		std::function<float()> ambientF_;
		std::function<float()> diffuseF_;
		std::function<float()> specularF_;
		std::function<float()> specularFocusF_;
	};

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
	std::vector<glm::vec4> colors;
	std::vector<glm::vec2> texCoord;
	std::vector<unsigned> indices;

	std::function<glm::mat4()> modelMatrixF;
	std::function<glm::vec4()> colorF;
	TextureComponentVariant texture;
	std::optional<ComponentId> renderingSetup;
	std::optional<Params3D> params3D;

	GLenum drawMode = GL_TRIANGLES;
	GLenum bufferDataUsage = GL_STATIC_DRAW;

	bool preserveTextureRatio = false;

	std::function<bool()> renderF = []() { return true; };

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
			? Tools::TransformMat4(vertices, getModelMatrix())
			: vertices;
	}

	virtual const std::vector<glm::vec4>& getColors() const
	{
		return colors;
	}

	virtual const std::vector<glm::vec2> getTexCoords(bool transformed = false) const
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

	virtual const std::vector<unsigned>& getIndices() const
	{
		return indices;
	}

	virtual glm::vec3 getOrigin() const
	{
		return getModelMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	glm::vec2 getOrigin2D() const
	{
		return getOrigin();
	}
};
