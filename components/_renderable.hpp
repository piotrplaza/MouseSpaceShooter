#pragma once

#include "_renderableDef.hpp"
#include "_componentBase.hpp"

#include "defaults.hpp"
#include <globals/components.hpp>

#include <tools/geometryHelpers.hpp>

#include <commonTypes/componentMappers.hpp>
#include <commonTypes/renderLayer.hpp>

#include <ogl/shaders/programBase.hpp>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <vector>
#include <deque>
#include <optional>

namespace Buffers
{
	struct GenericBuffers;
}

struct Renderable : ComponentBase, RenderableDef
{
	struct Instancing
	{
		Instancing& clearTransforms()
		{
			transforms_.clear();
			return *this;
		}

		Instancing& init(size_t count, const glm::mat4& value)
		{
			transforms_.clear();
			transforms_.resize(count, value);
			return *this;
		}

		Instancing& addTransform(const glm::mat4& value)
		{
			transforms_.push_back(value);
			return *this;
		}

		Instancing& addTransforms(const std::vector<glm::mat4>& value)
		{
			transforms_.insert(transforms_.end(), value.begin(), value.end());
			return *this;
		}

		std::vector<glm::mat4> transforms_;
	};

	Renderable() = default;

	Renderable(AbstractTextureComponentVariant texture,
		RenderingSetupF renderingSetupF,
		ShadersUtils::AccessorBase* customShadersProgram = nullptr,
		std::vector<glm::vec3> positions = {},
		std::vector<glm::vec2> texCoord = {}):
		RenderableDef(std::move(positions), std::move(texCoord), texture, std::move(renderingSetupF)),
		customShadersProgram(customShadersProgram)
	{
	}

	Renderable(std::vector<glm::vec3> positions, std::vector<glm::vec4> colors) :
		RenderableDef(std::move(positions), std::move(colors))
	{
	}

	CM::VP vpMatrix;
	ShadersUtils::AccessorBase* tfShaderProgram = nullptr;
	ShadersUtils::AccessorBase* customShadersProgram = nullptr;
	std::optional<Instancing> instancing;

	CM::RenderTexture targetTexture = Globals::Components().standardRenderTexture();
	RenderLayer renderLayer = Globals::Components().defaults().renderLayer;

	std::deque<RenderableDef> subsequence;
	unsigned subsequenceBegin = 0;
	unsigned posInSubsequence = 0;

	struct
	{
		CM::VP vpMatrix;
		Buffers::GenericBuffers* buffers = nullptr;
		Buffers::GenericBuffers* tfBuffers = nullptr;
	} loaded;
};
