#pragma once

#include <components/componentId.hpp>
#include <components/componentBase.hpp>
#include <components/typeComponentMappers.hpp>
#include <components/renderingBuffers.hpp>

#include <globals/components.hpp>

#include <commonTypes/buffersCollections.hpp>

#include <ogl/buffers/genericBuffers.hpp>

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

		template <typename SubComponent>
		void SubComponentToBuffers(const SubComponent& subComponent, Buffers::GenericSubBuffers& buffers)
		{
			buffers.allocateOrUpdateVerticesBuffer(subComponent.getVertices());
			buffers.allocateOrUpdateColorsBuffer(subComponent.getColors());
			buffers.allocateOrUpdateTexCoordBuffer(subComponent.getTexCoord());

			buffers.modelMatrixF = [&]() { return subComponent.getModelMatrix(); };
			buffers.renderingSetup = subComponent.renderingSetup;
			buffers.texture = subComponent.texture;
			buffers.drawMode = subComponent.drawMode;
			buffers.bufferDataUsage = subComponent.bufferDataUsage;
			buffers.preserveTextureRatio = subComponent.preserveTextureRatio;
			buffers.render = subComponent.render;
		}

		template <typename Component>
		void ComponentSubsequenceToSubBuffers(const Component& component, Buffers::GenericBuffers& buffers)
		{
			auto subBuffersIt = buffers.subsequence.begin();

			for (const auto& subComponent : component.subsequence)
			{
				auto& subBuffers = Details::ReuseOrEmplaceBack(buffers.subsequence, subBuffersIt);
				Details::SubComponentToBuffers(subComponent, subBuffers);
			}
		}

		template <typename Component>
		void ComponentToBuffers(Component& component, Buffers::GenericBuffers& buffers)
		{
			SubComponentToBuffers(component, buffers);

			buffers.customShadersProgram = component.customShadersProgram;
			buffers.resolutionMode = component.resolutionMode;
			buffers.posInSubsequence = component.posInSubsequence;
			buffers.sourceComponent = component.getComponentId();

			component.state = ComponentState::Ongoing;
		}
	}

	template <typename Component>
	inline void UpdateStaticBuffers(std::vector<Component>& components)
	{
		auto& staticBuffers = Globals::Components().renderingBuffers().staticBuffers;
		BuffersColections<std::vector<Buffers::GenericBuffers>::iterator> buffersIt;

		for (size_t layer = 0; layer < (size_t)RenderLayer::COUNT; ++layer)
		{
			buffersIt.basic[layer] = staticBuffers.basic[layer].end();
			buffersIt.textured[layer] = staticBuffers.textured[layer].end();
			buffersIt.customShaders[layer] = staticBuffers.customShaders[layer].end();
		}

		for(auto& component: components)
		{
			if (component.state == ComponentState::Outdated)
				continue;

			auto& selectedBuffers = [&, layer = (size_t)component.renderLayer]() -> auto& {
				if (component.customShadersProgram)
					return Details::ReuseOrEmplaceBack(staticBuffers.customShaders[layer], buffersIt.customShaders[layer]);
				else if (!std::holds_alternative<std::monostate>(component.texture))
					return Details::ReuseOrEmplaceBack(staticBuffers.textured[layer], buffersIt.textured[layer]);
				else
					return Details::ReuseOrEmplaceBack(staticBuffers.basic[layer], buffersIt.basic[layer]);
			}();

			if (component.getComponentId() == selectedBuffers.sourceComponent && component.state == ComponentState::Ongoing)
				continue;

			Details::ComponentToBuffers(component, selectedBuffers);
			Details::ComponentSubsequenceToSubBuffers(component, selectedBuffers);
		}

		for (size_t layer = 0; layer < (size_t)RenderLayer::COUNT; ++layer)
		{
			staticBuffers.basic[layer].resize(std::distance(staticBuffers.basic[layer].begin(), buffersIt.basic[layer]));
			staticBuffers.textured[layer].resize(std::distance(staticBuffers.textured[layer].begin(), buffersIt.textured[layer]));
			staticBuffers.customShaders[layer].resize(std::distance(staticBuffers.customShaders[layer].begin(), buffersIt.customShaders[layer]));
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

			Details::ComponentToBuffers(component, selectedBuffers);
			Details::ComponentSubsequenceToSubBuffers(component, selectedBuffers);
		}
	}
}
