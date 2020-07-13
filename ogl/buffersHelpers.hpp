#include <vector>
#include <unordered_map>

#include <GL/glew.h>

namespace Tools
{
	namespace Detail
	{
		template <typename Buffers>
		void AllocateOrUpdatePositionsData(Buffers& buffers, GLenum bufferDataUsage)
		{
			glBindBuffer(GL_ARRAY_BUFFER, buffers.positionBuffer);
			if (buffers.numOfAllocatedPositions < buffers.positionsCache.size())
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
		}

		template <typename Buffers>
		void AllocateOrUpdateTexCoordData(Buffers& buffers, GLenum bufferDataUsage)
		{
			glBindBuffer(GL_ARRAY_BUFFER, *buffers.texCoordBuffer);
			if (buffers.numOfAllocatedTexCoord < buffers.texCoordCache.size())
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
		}
	}

	template <typename Component, typename Buffers>
	inline void UpdateSimpleAndTexturesPositionsBuffers(const std::vector<Component>& components, Buffers& simpleBuffers,
		std::unordered_map<unsigned, Buffers>& texturesToBuffers, std::vector<Buffers>& customSimpleBuffers,
		std::vector<Buffers>& customTexturedBuffers, GLenum bufferDataUsage)
	{
		simpleBuffers.positionsCache.clear();
		for (auto& [texture, buffers] : texturesToBuffers) buffers.positionsCache.clear();
		for (auto& buffers: customSimpleBuffers) buffers.positionsCache.clear();
		for (auto& buffers: customTexturedBuffers) buffers.positionsCache.clear();

		for (auto& component : components)
		{
			const auto positionsCache = component.generatePositionsCache();
			auto& buffers = [&]() -> auto &
			{
				if (component.texture)
				{
					if (component.renderingSetup)
					{
						auto& buffers = customTexturedBuffers.emplace_back();
						buffers.renderingSetup = component.renderingSetup;
						buffers.texture = component.texture;
						return buffers;
					}
					else
					{
						auto& buffers = texturesToBuffers[*component.texture];
						buffers.texture = component.texture;
						return buffers;
					}
				}
				else
				{
					if (component.renderingSetup)
					{
						customSimpleBuffers.emplace_back().renderingSetup = component.renderingSetup;
						return customSimpleBuffers.back();
					}
					else
					{
						return simpleBuffers;
					}
				}
			}();

			buffers.positionsCache.insert(buffers.positionsCache.end(),
				positionsCache.begin(), positionsCache.end());
		}

		Detail::AllocateOrUpdatePositionsData(simpleBuffers, bufferDataUsage);
		for (auto& [texture, buffers] : texturesToBuffers) Detail::AllocateOrUpdatePositionsData(buffers, bufferDataUsage);
		for (auto& buffers : customSimpleBuffers) Detail::AllocateOrUpdatePositionsData(buffers, bufferDataUsage);
		for (auto& buffers : customTexturedBuffers) Detail::AllocateOrUpdatePositionsData(buffers, bufferDataUsage);
	}

	template <typename Component, typename Buffers>
	inline void UpdateTexCoordBuffers(const std::vector<Component>& components,
		std::unordered_map<unsigned, Buffers>& texturesToBuffers, std::vector<Buffers>& customTexturedBuffers,
		GLenum bufferDataUsage)
	{
		for (auto& [texture, buffers] : texturesToBuffers) buffers.texCoordCache.clear();
		for (auto& buffers : customTexturedBuffers) buffers.texCoordCache.clear();

		for (auto& component : components)
		{
			if (component.texture)
			{
				const auto positionsCache = component.generatePositionsCache(false);
				auto& texturedBuffers = component.renderingSetup
					? customTexturedBuffers.emplace_back()
					: texturesToBuffers[*component.texture];

				texturedBuffers.texture = component.texture;
				if (!texturedBuffers.texCoordBuffer) texturedBuffers.createTexCoordBuffer();

				for (const auto& position : positionsCache)
				{
					texturedBuffers.texCoordCache.emplace_back(position);
				}
			}
		}

		for (auto& [texture, buffers] : texturesToBuffers) Detail::AllocateOrUpdateTexCoordData(buffers, bufferDataUsage);
		for (auto& buffers : customTexturedBuffers) Detail::AllocateOrUpdateTexCoordData(buffers, bufferDataUsage);
	}
}
