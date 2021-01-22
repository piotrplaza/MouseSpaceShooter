#include <vector>
#include <unordered_map>

#include <GL/glew.h>

#include <componentId.hpp>
#include <componentBase.hpp>

namespace Tools
{
	namespace Detail
	{
		template <typename Buffers>
		void AllocateOrUpdatePositionsData(Buffers& buffers, GLenum bufferDataUsage)
		{
			glBindBuffer(GL_ARRAY_BUFFER, buffers.positionBuffer);
			if (buffers.numOfAllocatedPositions < buffers.positionsCache.size() || buffers.bufferDataUsage != bufferDataUsage)
			{
				glBufferData(GL_ARRAY_BUFFER, buffers.positionsCache.size()
					* sizeof(buffers.positionsCache.front()),
					buffers.positionsCache.data(), bufferDataUsage);
				buffers.numOfAllocatedPositions = buffers.positionsCache.size();
			}
			else
			{
				glBufferSubData(GL_ARRAY_BUFFER, 0, buffers.positionsCache.size()
					* sizeof(buffers.positionsCache.front()),
					buffers.positionsCache.data());
			}
			buffers.bufferDataUsage = bufferDataUsage;
		}

		template <typename Buffers>
		void AllocateOrUpdateTexCoordData(Buffers& buffers, GLenum bufferDataUsage)
		{
			glBindBuffer(GL_ARRAY_BUFFER, *buffers.texCoordBuffer);
			if (buffers.numOfAllocatedTexCoord < buffers.texCoordCache.size() || buffers.bufferDataUsage != bufferDataUsage)
			{
				glBufferData(GL_ARRAY_BUFFER, buffers.texCoordCache.size()
					* sizeof(buffers.texCoordCache.front()),
					buffers.texCoordCache.data(), bufferDataUsage);
				buffers.numOfAllocatedTexCoord = buffers.texCoordCache.size();
			}
			else
			{
				glBufferSubData(GL_ARRAY_BUFFER, 0, buffers.texCoordCache.size()
					* sizeof(buffers.texCoordCache.front()),
					buffers.texCoordCache.data());
			}
			buffers.bufferDataUsage = bufferDataUsage;
		}

		template <typename Buffers>
		Buffers& ReuseOrEmplaceBack(std::vector<Buffers>& buffers, typename std::vector<Buffers>::iterator& it)
		{
			return it == buffers.end()
				? buffers.emplace_back(), it = buffers.end(), buffers.back()
				: *it++;
		}
	}

	template <typename Component, typename Buffers>
	inline void UpdateTransformedPositionsBuffers(const std::vector<Component>& components, Buffers& simpleBuffers,
		std::unordered_map<unsigned, Buffers>& texturesToBuffers, std::vector<Buffers>& customSimpleBuffers,
		std::vector<Buffers>& customTexturedBuffers, std::vector<Buffers>& customShadersBuffers, GLenum bufferDataUsage)
	{
		simpleBuffers.positionsCache.clear();
		for (auto& [texture, buffers] : texturesToBuffers) buffers.positionsCache.clear();
		for (auto& buffers: customSimpleBuffers) buffers.positionsCache.clear();
		for (auto& buffers: customTexturedBuffers) buffers.positionsCache.clear();
		for (auto& buffers: customShadersBuffers) buffers.positionsCache.clear();

		auto customSimpleBuffersIt = customSimpleBuffers.begin();
		auto customTexturedBuffersIt = customTexturedBuffers.begin();
		auto customShadersBuffersIt = customShadersBuffers.begin();

		for (auto& component : components)
		{
			auto& buffers = [&]() -> auto&
			{
				if (component.customShadersProgram)
					return Detail::ReuseOrEmplaceBack(customShadersBuffers, customShadersBuffersIt);
				else if (component.texture)
				{
					if (component.renderingSetup)
						return Detail::ReuseOrEmplaceBack(customTexturedBuffers, customTexturedBuffersIt);
					else
						return texturesToBuffers[*component.texture];
				}
				else
				{
					if (component.renderingSetup)
						return Detail::ReuseOrEmplaceBack(customSimpleBuffers, customSimpleBuffersIt);
					else
						return simpleBuffers;
				}
			}();

			buffers.renderingSetup = component.renderingSetup.get();
			buffers.texture = component.texture;
			buffers.customShadersProgram = component.customShadersProgram;

			const auto& transformedPositions = component.getTransformedPositions();
			buffers.positionsCache.insert(buffers.positionsCache.end(),
				transformedPositions.begin(), transformedPositions.end());
		}

		customSimpleBuffers.resize(std::distance(customSimpleBuffers.begin(), customSimpleBuffersIt));
		customTexturedBuffers.resize(std::distance(customTexturedBuffers.begin(), customTexturedBuffersIt));
		customShadersBuffers.resize(std::distance(customShadersBuffers.begin(), customShadersBuffersIt));

		Detail::AllocateOrUpdatePositionsData(simpleBuffers, bufferDataUsage);
		for (auto& [texture, buffers] : texturesToBuffers) Detail::AllocateOrUpdatePositionsData(buffers, bufferDataUsage);
		for (auto& buffers : customSimpleBuffers) Detail::AllocateOrUpdatePositionsData(buffers, bufferDataUsage);
		for (auto& buffers : customTexturedBuffers) Detail::AllocateOrUpdatePositionsData(buffers, bufferDataUsage);
		for (auto& buffers : customShadersBuffers) Detail::AllocateOrUpdatePositionsData(buffers, bufferDataUsage);
	}

	template <typename Component, typename Buffers>
	inline void UpdatePositionsBuffers(const std::vector<Component>& components, std::vector<Buffers>& simpleBuffers,
		std::vector<Buffers>& texturedBuffers, std::vector<Buffers>& customShadersBuffers, GLenum bufferDataUsage)
	{
		auto simpleBuffersIt = simpleBuffers.begin();
		auto texturedBuffersIt = texturedBuffers.begin();
		auto customShadersBuffersIt = customShadersBuffers.begin();

		for (auto& component : components)
		{
			auto& buffers = [&]() -> auto&
			{
				if (component.customShadersProgram)
					return Detail::ReuseOrEmplaceBack(customShadersBuffers, customShadersBuffersIt);
				else if (component.texture)
					return Detail::ReuseOrEmplaceBack(texturedBuffers, texturedBuffersIt);
				else
					return Detail::ReuseOrEmplaceBack(simpleBuffers, simpleBuffersIt);
			}();

			const auto& positions = component.getPositions();
			buffers.renderingSetup = component.renderingSetup.get();
			buffers.texture = component.texture;
			buffers.animationController = component.animationController.get();
			buffers.customShadersProgram = component.customShadersProgram;
			buffers.positionsCache.clear();
			buffers.positionsCache.insert(buffers.positionsCache.end(), positions.begin(), positions.end());
			Detail::AllocateOrUpdatePositionsData(buffers, bufferDataUsage);
		}

		simpleBuffers.resize(std::distance(simpleBuffers.begin(), simpleBuffersIt));
		texturedBuffers.resize(std::distance(texturedBuffers.begin(), texturedBuffersIt));
		customShadersBuffers.resize(std::distance(customShadersBuffers.begin(), customShadersBuffersIt));
	}

	template <typename Component, typename Buffers>
	inline void UpdateTexCoordBuffers(const std::vector<Component>& components,
		std::unordered_map<unsigned, Buffers>& texturesToBuffers, std::vector<Buffers>& customTexturedBuffers,
		std::vector<Buffers>& customShadersTexturedBuffers, GLenum bufferDataUsage)
	{
		for (auto& [texture, buffers] : texturesToBuffers) buffers.texCoordCache.clear();
		for (auto& buffers : customTexturedBuffers) buffers.texCoordCache.clear();
		for (auto& buffers : customShadersTexturedBuffers) buffers.texCoordCache.clear();

		auto customTexturedBuffersIt = customTexturedBuffers.begin();
		auto customShadersBuffersIt = customShadersTexturedBuffers.begin();

		for (auto& component : components)
		{
			if (component.texture)
			{
				auto& buffers = [&]() -> auto&
				{
					if (component.customShadersProgram)
						return Detail::ReuseOrEmplaceBack(customShadersTexturedBuffers, customShadersBuffersIt);
					else if (component.renderingSetup)
						return Detail::ReuseOrEmplaceBack(customTexturedBuffers, customTexturedBuffersIt);
					else
						return texturesToBuffers[*component.texture];
				}();

				if (!buffers.texCoordBuffer) buffers.createTexCoordBuffer();
				const auto& texCoord = component.getTexCoord();
				buffers.texCoordCache.insert(buffers.texCoordCache.end(), texCoord.begin(), texCoord.end());
				buffers.textureRatioPreserved = component.isTextureRatioPreserved();
			}
		}

		for (auto& [texture, buffers] : texturesToBuffers) Detail::AllocateOrUpdateTexCoordData(buffers, bufferDataUsage);
		for (auto& buffers : customTexturedBuffers) Detail::AllocateOrUpdateTexCoordData(buffers, bufferDataUsage);
		for (auto& buffers : customShadersTexturedBuffers) Detail::AllocateOrUpdateTexCoordData(buffers, bufferDataUsage);
	}

	template <typename Component, typename Buffers>
	inline void UpdateTexCoordBuffers(const std::vector<Component>& components,
		std::vector<Buffers>& texturedBuffers, std::vector<Buffers>& customShadersTexturedBuffers, GLenum bufferDataUsage)
	{
		auto texturedBuffersIt = texturedBuffers.begin();
		auto customShadersTexturedBuffersIt = customShadersTexturedBuffers.begin();

		for (const auto& component : components)
		{
			if (component.texture)
			{
				auto& relevantBuffers = component.customShadersProgram
					? customShadersTexturedBuffers
					: texturedBuffers;
				auto& relevantBuffersIt = component.customShadersProgram
					? customShadersTexturedBuffersIt
					: texturedBuffersIt;

				auto& buffers = Detail::ReuseOrEmplaceBack(relevantBuffers, relevantBuffersIt);
				if (!buffers.texCoordBuffer) buffers.createTexCoordBuffer();
				buffers.texCoordCache = component.getTexCoord();
				buffers.textureRatioPreserved = component.isTextureRatioPreserved();
				Detail::AllocateOrUpdateTexCoordData(buffers, bufferDataUsage);
			}
		}
	}

	template <typename Component, typename Buffers>
	inline void UpdatePosTexCoordBuffers(std::unordered_map<ComponentId, Component>& components, std::unordered_map<ComponentId, Buffers>& simpleBuffers,
		std::unordered_map<ComponentId, Buffers>& texturedBuffers, std::unordered_map<ComponentId, Buffers>& customShadersBuffers, GLenum bufferDataUsage)
	{
		for(auto& [id, component] : components)
		{
			if (component.state == ComponentState::Ongoing)
				continue;
			else
			{
				auto& mapOfBuffers = [&]() -> auto&
				{
					if (component.customShadersProgram)
						return customShadersBuffers;
					else if (component.texture)
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
				buffers.renderingSetup = component.renderingSetup.get();
				buffers.texture = component.texture;
				buffers.animationController = component.animationController.get();
				buffers.customShadersProgram = component.customShadersProgram;
				buffers.positionsCache = component.getPositions();

				Detail::AllocateOrUpdatePositionsData(buffers, bufferDataUsage);

				if (component.texture)
				{
					if (!buffers.texCoordBuffer) buffers.createTexCoordBuffer();
					buffers.texCoordCache = component.getTexCoord();
					buffers.textureRatioPreserved = component.isTextureRatioPreserved();
					Detail::AllocateOrUpdateTexCoordData(buffers, bufferDataUsage);
				}

				component.state = ComponentState::Ongoing;
			}
		}
	}
}
