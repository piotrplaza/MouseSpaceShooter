#pragma once

#include "_renderableDef.hpp"
#include "_componentBase.hpp"

#include <tools/geometryHelpers.hpp>

#include <commonTypes/typeComponentMappers.hpp>
#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/renderLayer.hpp>

#include <ogl/shaders.hpp>

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

	Renderable(TextureComponentVariant texture,
		std::optional<ComponentId> renderingSetup,
		RenderLayer renderLayer,
		std::optional<Shaders::ProgramId> customShadersProgram,
		std::vector<glm::vec3> vertices = {},
		std::vector<glm::vec2> texCoord = {}):
		RenderableDef(std::move(vertices), std::move(texCoord), texture, renderingSetup),
		renderLayer(renderLayer),
		customShadersProgram(customShadersProgram)
	{
	}

	std::function<void()> stepF;

	virtual void step()
	{
		if (stepF)
			stepF();
	}

	std::optional<Shaders::ProgramId> customShadersProgram;
	std::optional<Instancing> instancing;

	ResolutionMode resolutionMode = ResolutionMode::Normal;
	RenderLayer renderLayer = RenderLayer::Midground;

	std::deque<RenderableDef> subsequence;
	unsigned subsequenceBegin = 0;
	unsigned posInSubsequence = 0;

	struct
	{
		Buffers::GenericBuffers* buffers = nullptr;
	} loaded;

	void setEnable(bool value) override
	{
		ComponentBase::setEnable(value);
		renderF = [value]() { return value; };
		for (auto& e : subsequence)
			e.renderF = [value]() { return value; };
	}
};
