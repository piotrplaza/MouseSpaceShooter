#include <vector>
#include <unordered_map>

#include <GL/glew.h>

namespace Tools
{
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
			if (component.texture)
			{
				auto& texturedBuffers = texturesToBuffers[*component.texture];
				texturedBuffers.positionsCache.insert(texturedBuffers.positionsCache.end(),
					positionsCache.begin(), positionsCache.end());
			}
			else
			{
				auto& bufferPositionsCache = [&]() -> auto&
				{
					if (component.renderingSetup)
					{
						customSimpleBuffers.emplace_back().renderingSetup = component.renderingSetup;
						return customSimpleBuffers.back().positionsCache;
					}
					else
					{
						return simpleBuffers.positionsCache;
					}
				}();
				
				bufferPositionsCache.insert(bufferPositionsCache.end(),
					positionsCache.begin(), positionsCache.end());

			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, simpleBuffers.positionBuffer);
		if (simpleBuffers.numOfAllocatedVertices < simpleBuffers.positionsCache.size())
		{
			glBufferData(GL_ARRAY_BUFFER, simpleBuffers.positionsCache.size()
				* sizeof(simpleBuffers.positionsCache.front()),
				simpleBuffers.positionsCache.data(), bufferDataUsage);
			simpleBuffers.numOfAllocatedVertices = simpleBuffers.positionsCache.size();
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, simpleBuffers.positionsCache.size()
				* sizeof(simpleBuffers.positionsCache.front()),
				simpleBuffers.positionsCache.data());
		}

		for (auto& [texture, texturedBuffers] : texturesToBuffers)
		{
			glBindBuffer(GL_ARRAY_BUFFER, texturedBuffers.positionBuffer);
			if (texturedBuffers.numOfAllocatedVertices < texturedBuffers.positionsCache.size())
			{
				glBufferData(GL_ARRAY_BUFFER, texturedBuffers.positionsCache.size()
					* sizeof(texturedBuffers.positionsCache.front()),
					texturedBuffers.positionsCache.data(), bufferDataUsage);
				texturedBuffers.numOfAllocatedVertices = texturedBuffers.positionsCache.size();
			}
			else
			{
				glBufferSubData(GL_ARRAY_BUFFER, 0, texturedBuffers.positionsCache.size()
					* sizeof(texturedBuffers.positionsCache.front()),
					texturedBuffers.positionsCache.data());
			}
		}

		for (auto& customSimpleBuffer : customSimpleBuffers)
		{
			glBindBuffer(GL_ARRAY_BUFFER, customSimpleBuffer.positionBuffer);
			if (customSimpleBuffer.numOfAllocatedVertices < customSimpleBuffer.positionsCache.size())
			{
				glBufferData(GL_ARRAY_BUFFER, customSimpleBuffer.positionsCache.size()
					* sizeof(customSimpleBuffer.positionsCache.front()),
					customSimpleBuffer.positionsCache.data(), bufferDataUsage);
				customSimpleBuffer.numOfAllocatedVertices = customSimpleBuffer.positionsCache.size();
			}
			else
			{
				glBufferSubData(GL_ARRAY_BUFFER, 0, customSimpleBuffer.positionsCache.size()
					* sizeof(customSimpleBuffer.positionsCache.front()),
					customSimpleBuffer.positionsCache.data());
			}
		}
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

				if (!texturedBuffers.texCoordBuffer) texturedBuffers.createTexCoordBuffer();

				for (const auto& position : positionsCache)
				{
					texturedBuffers.texCoordCache.emplace_back(position);
				}
			}
		}

		for (auto& [texture, buffers] : texturesToBuffers)
		{
			glBindBuffer(GL_ARRAY_BUFFER, *buffers.texCoordBuffer);
			glBufferData(GL_ARRAY_BUFFER, buffers.texCoordCache.size() * sizeof(buffers.texCoordCache.front()),
				buffers.texCoordCache.data(), bufferDataUsage);
		}
		for (auto& buffers : customTexturedBuffers)
		{
			glBindBuffer(GL_ARRAY_BUFFER, *buffers.texCoordBuffer);
			glBufferData(GL_ARRAY_BUFFER, buffers.texCoordCache.size() * sizeof(buffers.texCoordCache.front()),
				buffers.texCoordCache.data(), bufferDataUsage);
		}
	}
}
