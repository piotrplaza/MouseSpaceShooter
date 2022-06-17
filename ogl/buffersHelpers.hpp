#pragma once

#include <components/componentId.hpp>
#include <components/componentBase.hpp>
#include <components/typeComponentMappers.hpp>

#include <GL/glew.h>

#include <vector>
#include <unordered_map>

namespace Tools
{
	namespace Details
	{
		template <typename Buffers>
		Buffers& ReuseOrEmplaceBack(std::vector<Buffers>& buffers, typename std::vector<Buffers>::iterator& it)
		{
			return it == buffers.end()
				? buffers.emplace_back(), it = buffers.end(), buffers.back()
				: *it++;
		}

		template <typename Component, typename Buffers>
		void ComponentToBuffers(Component& component, Buffers& buffers)
		{
			buffers.allocateOrUpdatePositionsBuffer(component.getVertices());

			if (!std::holds_alternative<std::monostate>(component.texture))
				buffers.allocateOrUpdateTexCoordBuffer(component.getTexCoord());

			buffers.modelMatrixF = [&]() { return component.getModelMatrix(); };
			buffers.renderingSetup = component.renderingSetup;
			buffers.texture = component.texture;
			buffers.customShadersProgram = component.customShadersProgram;
			buffers.resolutionMode = component.resolutionMode;
			buffers.drawMode = component.drawMode;
			buffers.bufferDataUsage = component.bufferDataUsage;
			buffers.preserveTextureRatio = component.preserveTextureRatio;

			component.state = ComponentState::Ongoing;
		}
	}

	template <typename Component, typename Buffers>
	inline void UpdateStaticBuffers(std::vector<Component>& components,
		std::vector<Buffers>& simpleBuffers, std::vector<Buffers>& texturedBuffers, std::vector<Buffers>& customShadersBuffers)
	{
		auto simpleBuffersIt = simpleBuffers.begin();
		auto texturedBuffersIt = texturedBuffers.begin();
		auto customShadersBuffersIt = customShadersBuffers.begin();

		Globals::ForEach(components, [&](auto& component) {
			auto& buffers = [&]() -> auto& {
				if (component.customShadersProgram)
					return Details::ReuseOrEmplaceBack(customShadersBuffers, customShadersBuffersIt);
				else if (!std::holds_alternative<std::monostate>(component.texture))
					return Details::ReuseOrEmplaceBack(texturedBuffers, texturedBuffersIt);
				else
					return Details::ReuseOrEmplaceBack(simpleBuffers, simpleBuffersIt);
			}();

			Details::ComponentToBuffers(component, buffers);
			});

		simpleBuffers.resize(std::distance(simpleBuffers.begin(), simpleBuffersIt));
		texturedBuffers.resize(std::distance(texturedBuffers.begin(), texturedBuffersIt));
		customShadersBuffers.resize(std::distance(customShadersBuffers.begin(), customShadersBuffersIt));
	}

	template <typename Component, typename Buffers>
	inline void UpdateDynamicBuffers(std::unordered_map<ComponentId, Component>& components, std::unordered_map<ComponentId, Buffers>& simpleBuffers,
		std::unordered_map<ComponentId, Buffers>& texturedBuffers, std::unordered_map<ComponentId, Buffers>& customShadersBuffers)
	{
		for(auto& [id, component] : components)
		{
			if (component.state == ComponentState::Ongoing)
				continue;

			auto& mapOfBuffers = [&]() -> auto&
			{
				if (component.customShadersProgram)
					return customShadersBuffers;
				else if (!std::holds_alternative<std::monostate>(component.texture))
					return texturedBuffers;
				else
					return simpleBuffers;
			}();

			if (component.state == ComponentState::Outdated)
			{
				mapOfBuffers.erase(id);
				continue;
			}

			auto& buffers = mapOfBuffers[id];

			Details::ComponentToBuffers(component, buffers);
		}
	}
}
