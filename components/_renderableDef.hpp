#pragma once

#include <tools/geometryHelpers.hpp>

#include <commonTypes/componentMappers.hpp>
#include <commonTypes/fTypes.hpp>

#include <ogl/shadersUtils.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <optional>

namespace Buffers
{
	struct GenericSubBuffers;
}

struct RenderableDef
{
	using RenderingSetupF = std::function<std::function<void()>(ShadersUtils::ProgramId)>;

	struct Params3D
	{
		Params3D& addNormals(const std::vector<glm::vec3>& value)
		{
			normals_.insert(normals_.end(), value.begin(), value.end());
			return *this;
		}

		Params3D& ambient(float value)
		{
			ambient_ = value;
			return *this;
		}

		Params3D& diffuse(float value)
		{
			diffuse_ = value;
			return *this;
		}

		Params3D& specular(float value)
		{
			specular_ = value;
			return *this;
		}

		Params3D& specularFocus(float value)
		{
			specularFocus_ = value;
			return *this;
		}

		Params3D& specularMaterialColorFactor(float value)
		{
			specularMaterialColorFactor_ = value;
			return *this;
		}

		Params3D& illuminationF(FVec4 value)
		{
			illuminationF_ = std::move(value);
			return *this;
		}

		Params3D& darkColor(FVec3 value)
		{
			darkColor_ = std::move(value);
			return *this;
		}

		Params3D& lightModelEnabled(bool value)
		{
			lightModelEnabled_ = value;
			return *this;
		}

		Params3D& alphaDiscardTreshold(float value)
		{
			alphaDiscardTreshold_ = value;
			return *this;
		}

		Params3D& gpuSideInstancedNormalTransforms(bool value)
		{
			gpuSideInstancedNormalTransforms_ = value;
			return *this;
		}

		Params3D& fogAmplification(float value)
		{
			fogAmplification_ = value;
			return *this;
		}

		Params3D& lightModelColorNormalization(bool value)
		{
			lightModelColorNormalization_ = value;
			return *this;
		}

		std::vector<glm::vec3> normals_;
		float ambient_ = 0.1f;
		float diffuse_ = 0.8f;
		float specular_ = 3.0f;
		float specularFocus_ = 8.0f;
		float specularMaterialColorFactor_ = 0.0f;
		FVec4 illuminationF_;
		FVec3 darkColor_;
		bool lightModelEnabled_ = true;
		float alphaDiscardTreshold_ = 0.1f;
		bool gpuSideInstancedNormalTransforms_ = false;
		float fogAmplification_ = 0.0f;
		bool lightModelColorNormalization_ = false;
	};

	RenderableDef() = default;

	RenderableDef(std::vector<glm::vec3> positions,
		std::vector<glm::vec2> texCoord = {},
		AbstractTextureComponentVariant texture = std::monostate{},
		RenderingSetupF renderingSetupF = nullptr) :
		positions(std::move(positions)),
		texCoord(std::move(texCoord)),
		texture(texture),
		renderingSetupF(std::move(renderingSetupF))
	{
	}

	RenderableDef(std::vector<glm::vec3> positions, std::vector<glm::vec4> colors) :
		positions(std::move(positions)),
		colors(std::move(colors))
	{
	}

	std::vector<glm::vec3> positions;
	std::vector<glm::vec4> colors;
	std::vector<glm::vec2> texCoord;
	std::vector<unsigned> indices;

	RenderingSetupF renderingSetupF;
	FMat4 modelMatrixF = glm::mat4(1.0f);
	FVec3 originF = [&]() { return modelMatrixF() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); };
	FVec4 colorF;
	AbstractTextureComponentVariant texture;
	std::optional<Params3D> params3D;

	GLenum drawMode = GL_TRIANGLES;
	GLenum bufferDataUsage = GL_STATIC_DRAW;

	FBool renderF = true;

	struct
	{
		Buffers::GenericSubBuffers* subBuffers = nullptr;
	} loaded;

	virtual std::vector<glm::vec3> getPositions(bool transformed = false) const
	{
		return transformed
			? Tools::TransformMat4(positions, modelMatrixF())
			: positions;
	}

	virtual std::vector<glm::vec3>& accessPositions()
	{
		return positions;
	}

	virtual const std::vector<glm::vec4>& getColors() const
	{
		return colors;
	}

	virtual std::vector < glm::vec4>& accessColors()
	{
		return colors;
	}

	virtual const std::vector<glm::vec2> getTexCoords(bool transformed = false) const
	{
		const auto positions = getPositions(transformed);
		if (texCoord.empty())
		{
			return std::vector<glm::vec2>(positions.begin(), positions.end());
		}
		else if (texCoord.size() < positions.size())
		{
			std::vector<glm::vec2> cyclicTexCoord;
			cyclicTexCoord.reserve(positions.size());
			for (size_t i = 0; i < positions.size(); ++i)
				cyclicTexCoord.push_back(texCoord[i % texCoord.size()]);
			return cyclicTexCoord;
		}
		else
		{
			assert(texCoord.size() == positions.size());
			return texCoord;
		}
	}

	virtual std::vector<glm::vec2>& accessTexCoords()
	{
		return texCoord;
	}

	virtual const std::vector<unsigned>& getIndices() const
	{
		return indices;
	}

	virtual std::vector<unsigned>& accessIndices()
	{
		return indices;
	}

	glm::vec2 getOrigin2D() const
	{
		return originF();
	}
};
