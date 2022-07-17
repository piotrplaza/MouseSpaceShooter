#pragma once

#include <components/renderingBuffers.hpp>

#include <globals/components.hpp>

#include <ranges>

namespace Tools
{
	template <typename Component>
	inline void UpdateStaticBuffers(std::vector<Component>& components, size_t offset = 0)
	{
		auto& staticBuffers = Globals::Components().renderingBuffers().staticBuffers;

		for(auto& component: components | std::views::drop(offset))
		{
			if (component.state == ComponentState::Outdated)
				continue;

			auto& selectedBuffers = [&, layer = (size_t)component.renderLayer]() -> auto& {
				if (component.customShadersProgram)
					return staticBuffers.customShaders[layer].emplace_back();
				else if (!std::holds_alternative<std::monostate>(component.texture))
					return staticBuffers.textured[layer].emplace_back();
				else
					return staticBuffers.basic[layer].emplace_back();
			}();

			selectedBuffers.applyComponent(component);
			selectedBuffers.applyComponentSubsequence(component);
		}
	}

	template <typename Component>
	inline void UpdateDynamicBuffers(std::unordered_map<ComponentId, Component>& components)
	{
		auto& dynamicBuffers = Globals::Components().renderingBuffers().dynamicBuffers;

		for(auto& [id, component] : components)
		{
			if (component.state == ComponentState::Ongoing)
				continue;

			const auto layer = (size_t)component.renderLayer;

			auto& mapOfSelectedBuffers = [&]() -> auto&
			{
				if (component.customShadersProgram)
					return dynamicBuffers.customShaders[layer];
				else if (!std::holds_alternative<std::monostate>(component.texture))
					return dynamicBuffers.textured[layer];
				else
					return dynamicBuffers.basic[layer];
			}();

			if (component.state == ComponentState::Outdated)
			{
				mapOfSelectedBuffers.erase(id);
				continue;
			}

			auto& selectedBuffers = mapOfSelectedBuffers[id];

			selectedBuffers.applyComponent(component);
			selectedBuffers.applyComponentSubsequence(component);
		}
	}
}
