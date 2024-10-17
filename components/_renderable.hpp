#pragma once

#include "_renderableDef.hpp"
#include "_componentBase.hpp"

#include "defaults.hpp"
#include <globals/components.hpp>

#include <tools/geometryHelpers.hpp>

#include <commonTypes/componentMappers.hpp>
#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/renderLayer.hpp>

#include <ogl/shadersUtils.hpp>

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
		RenderLayer renderLayer,
		std::optional<ShadersUtils::ProgramId> customShadersProgram,
		std::vector<glm::vec3> vertices = {},
		std::vector<glm::vec2> texCoord = {}):
		RenderableDef(std::move(vertices), std::move(texCoord), texture, std::move(renderingSetupF)),
		renderLayer(renderLayer),
		customShadersProgram(customShadersProgram)
	{
	}

	std::optional<ShadersUtils::ProgramId> customShadersProgram;
	std::optional<Instancing> instancing;

	ResolutionMode resolutionMode = Globals::Components().defaults().resolutionMode;
	RenderLayer renderLayer = Globals::Components().defaults().renderLayer;

	std::deque<RenderableDef> subsequence;
	unsigned subsequenceBegin = 0;
	unsigned posInSubsequence = 0;

	struct
	{
		Buffers::GenericBuffers* buffers = nullptr;
	} loaded;
};
