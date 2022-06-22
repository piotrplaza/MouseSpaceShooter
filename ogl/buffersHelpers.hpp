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

		template <typename SubComponent, typename Buffers>
		void SubComponentToBuffers(const SubComponent& subComponent, Buffers& buffers)
		{
			buffers.allocateOrUpdateVerticesBuffer(subComponent.getVertices());

			if (!std::holds_alternative<std::monostate>(subComponent.texture))
				buffers.allocateOrUpdateTexCoordBuffer(subComponent.getTexCoord());

			buffers.modelMatrixF = [&]() { return subComponent.getModelMatrix(); };
			buffers.renderingSetup = subComponent.renderingSetup;
			buffers.texture = subComponent.texture;
			buffers.drawMode = subComponent.drawMode;
			buffers.bufferDataUsage = subComponent.bufferDataUsage;
			buffers.preserveTextureRatio = subComponent.preserveTextureRatio;
			buffers.render = subComponent.render;
		}

		template <typename Component, typename Buffers>
		void ComponentSubsequenceToSubbuffers(const Component& component, Buffers& buffers)
		{
			auto subBuffersIt = buffers.subsequence.begin();

			for (const auto& subComponent : component.subsequence)
			{
				auto& subBuffers = Details::ReuseOrEmplaceBack(buffers.subsequence, subBuffersIt);
				Details::SubComponentToBuffers(subComponent, subBuffers);
			}
		}

		template <typename Component, typename Buffers>
		void ComponentToBuffers(Component& component, Buffers& buffers)
		{
			SubComponentToBuffers(component, buffers);

			buffers.customShadersProgram = component.customShadersProgram;
			buffers.resolutionMode = component.resolutionMode;
			buffers.posInSubsequence = component.posInSubsequence;
			buffers.sourceComponent = component.getComponentId();

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

		auto prepareBuffersLocation = [&](const Component& component) -> auto& {
			if (component.customShadersProgram)
				return Details::ReuseOrEmplaceBack(customShadersBuffers, customShadersBuffersIt);
			else if (!std::holds_alternative<std::monostate>(component.texture))
				return Details::ReuseOrEmplaceBack(texturedBuffers, texturedBuffersIt);
			else
				return Details::ReuseOrEmplaceBack(simpleBuffers, simpleBuffersIt);
		};

		Globals::ForEach(components, [&](auto& component) {
			if (component.state == ComponentState::Outdated)
				return;

			auto& buffers = prepareBuffersLocation(component);

			if (component.getComponentId() == buffers.sourceComponent && component.state == ComponentState::Ongoing)
				return;

			Details::ComponentToBuffers(component, buffers);
			Details::ComponentSubsequenceToSubbuffers(component, buffers);
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
			Details::ComponentSubsequenceToSubbuffers(component, buffers);
		}
	}
}
