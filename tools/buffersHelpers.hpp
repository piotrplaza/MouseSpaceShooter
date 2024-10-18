#pragma once

#include <components/renderingBuffers.hpp>

#include <globals/components.hpp>

#include <commonTypes/componentsContainers.hpp>

#include <ranges>

namespace Tools
{
	template <typename Component>
	inline void UpdateStaticBuffers(StaticComponents<Component>& components, size_t offset = 0)
	{
		auto& staticBuffers = Globals::Components().renderingBuffers().staticBuffers;

		for (auto& component : components.underlyingContainer() | std::views::drop(offset))
		{
			if (component.state == ComponentState::Ongoing)
				continue;

			auto& selectedBuffers = [&, layer = (size_t)component.renderLayer]() -> auto& {
				if (component.customShadersProgram)
					return staticBuffers.customShaders[layer].emplace_back();
				else if (component.params3D)
				{
					if (std::holds_alternative<std::monostate>(component.texture))
						return staticBuffers.basicPhong[layer].emplace_back();
					else
						return staticBuffers.texturedPhong[layer].emplace_back();
				}
				else if (std::holds_alternative<std::monostate>(component.texture))
					return staticBuffers.basic[layer].emplace_back();
				else
					return staticBuffers.textured[layer].emplace_back();
			}();

			selectedBuffers.applyComponent(component, true);
		}
	}

	template <typename Component>
	inline void UpdateDynamicBuffers(DynamicComponents<Component>& components)
	{
		auto& dynamicBuffers = Globals::Components().renderingBuffers().dynamicBuffers;

		for (auto& component : components)
		{
			if (component.state == ComponentState::Ongoing)
				continue;

			// TODO: What if the layer or shader type was changed? Prob leak.

			const auto layer = (size_t)component.renderLayer;

			auto& mapOfSelectedBuffers = [&]() -> auto&
			{
				if (component.customShadersProgram)
					return dynamicBuffers.customShaders[layer];
				else if (component.params3D)
				{
					if (std::holds_alternative<std::monostate>(component.texture))
						return dynamicBuffers.basicPhong[layer];
					else
						return dynamicBuffers.texturedPhong[layer];
				}
				else if (std::holds_alternative<std::monostate>(component.texture))
					return dynamicBuffers.basic[layer];
				else
					return dynamicBuffers.textured[layer];
			}();

			auto& selectedBuffers = mapOfSelectedBuffers[component.getComponentId()];

			selectedBuffers.applyComponent(component, false);

			component.teardownF = [&]() { mapOfSelectedBuffers.erase(component.getComponentId()); };
		}
	}

	struct SubImageData
	{
		const float* data;
		glm::ivec2 size;
		glm::ivec2 offset;
	};

	SubImageData GetSubImage(
		const float* sourceImageData,
		const glm::ivec2& sourceImageSize,
		const glm::ivec2& offset,
		const glm::ivec2& subImageSize,
		int numOfChannels,
		std::vector<float>& operationalBuffer);

	SubImageData ClipSubImage(
		const float* subImageData,
		const glm::ivec2& subImagebSize,
		const glm::ivec2& offset,
		const glm::ivec2& targetImageSize,
		const int numOfChannels,
		std::vector<float>& operationalBuffer);
}
