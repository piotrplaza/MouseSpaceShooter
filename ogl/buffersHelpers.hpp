#pragma once

#include <components/componentId.hpp>
#include <components/componentBase.hpp>
#include <components/typeComponentMappers.hpp>

#include <GL/glew.h>

#include <vector>
#include <unordered_map>

namespace Tools
{
	namespace Detail
	{
		template <typename Buffers>
		Buffers& ReuseOrEmplaceBack(std::vector<Buffers>& buffers, typename std::vector<Buffers>::iterator& it)
		{
			return it == buffers.end()
				? buffers.emplace_back(), it = buffers.end(), buffers.back()
				: *it++;
		}
	}

	template <typename Component, typename Buffers>
	inline void UpdatePositionsBuffers(const std::vector<Component>& components,
		std::vector<Buffers>& simpleBuffers, std::vector<Buffers>& texturedBuffers, std::vector<Buffers>& customShadersBuffers)
	{
		auto simpleBuffersIt = simpleBuffers.begin();
		auto texturedBuffersIt = texturedBuffers.begin();
		auto customShadersBuffersIt = customShadersBuffers.begin();

		Globals::ForEach(components, [&](const auto& component) {
			auto& buffers = [&]() -> auto& {
				if (component.customShadersProgram)
					return Detail::ReuseOrEmplaceBack(customShadersBuffers, customShadersBuffersIt);
				else if (!std::holds_alternative<std::monostate>(component.texture))
					return Detail::ReuseOrEmplaceBack(texturedBuffers, texturedBuffersIt);
				else
					return Detail::ReuseOrEmplaceBack(simpleBuffers, simpleBuffersIt);
			}();

			buffers.allocateOrUpdatePositionsBuffer(component.getVertexPositions());
			buffers.modelMatrixF = [&]() { return component.getModelMatrix(); };
			buffers.renderingSetup = component.renderingSetup;
			buffers.texture = component.texture;
			buffers.customShadersProgram = component.customShadersProgram;
			buffers.resolutionMode = component.resolutionMode;
			buffers.drawMode = component.drawMode;
			buffers.bufferDataUsage = component.bufferDataUsage;
			});

		simpleBuffers.resize(std::distance(simpleBuffers.begin(), simpleBuffersIt));
		texturedBuffers.resize(std::distance(texturedBuffers.begin(), texturedBuffersIt));
		customShadersBuffers.resize(std::distance(customShadersBuffers.begin(), customShadersBuffersIt));
	}

	template <typename Component, typename Buffers>
	inline void UpdateTexCoordBuffers(const std::vector<Component>& components,
		std::vector<Buffers>& texturedBuffers, std::vector<Buffers>& customShadersTexturedBuffers)
	{
		auto texturedBuffersIt = texturedBuffers.begin();
		auto customShadersTexturedBuffersIt = customShadersTexturedBuffers.begin();

		Globals::ForEach(components, [&](const auto& component) {
			if (!std::holds_alternative<std::monostate>(component.texture))
			{
				auto& relevantBuffers = component.customShadersProgram
					? customShadersTexturedBuffers
					: texturedBuffers;
				auto& relevantBuffersIt = component.customShadersProgram
					? customShadersTexturedBuffersIt
					: texturedBuffersIt;

				auto& buffers = Detail::ReuseOrEmplaceBack(relevantBuffers, relevantBuffersIt);
				buffers.allocateOrUpdateTexCoordBuffer(component.getTexCoord());
				buffers.preserveTextureRatio = component.preserveTextureRatio;
				buffers.drawMode = component.drawMode;
				buffers.bufferDataUsage = component.bufferDataUsage;
			}
			});
	}

	template <typename Component, typename Buffers>
	inline void UpdatePosTexCoordBuffers(std::unordered_map<ComponentId, Component>& components, std::unordered_map<ComponentId, Buffers>& simpleBuffers,
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
			buffers.allocateOrUpdatePositionsBuffer(component.getVertexPositions());
			buffers.renderingSetup = component.renderingSetup;
			buffers.texture = component.texture;
			buffers.customShadersProgram = component.customShadersProgram;
			buffers.resolutionMode = component.resolutionMode;
			buffers.drawMode = component.drawMode;
			buffers.bufferDataUsage = component.bufferDataUsage;

			if (!std::holds_alternative<std::monostate>(component.texture))
			{
				buffers.allocateOrUpdateTexCoordBuffer(component.getTexCoord());
				buffers.preserveTextureRatio = component.preserveTextureRatio;
			}

			component.state = ComponentState::Ongoing;
		}
	}
}
