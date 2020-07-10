#include <vector>
#include <unordered_map>

#include <GL/glew.h>

namespace Tools
{
	template <typename Component, typename Buffers>
	void UpdateSimpleAndTexturesPositionsBuffers(const std::vector<Component>& components, Buffers& simpleBuffers,
		std::unordered_map<unsigned, Buffers>& texturesToBuffers, GLenum bufferDataUsage)
	{
		simpleBuffers.positionsCache.clear();
		for (auto& [texture, buffers] : texturesToBuffers)
		{
			buffers.positionsCache.clear();
		}

		for (auto& component : components)
		{
			const auto positionsCache = component.generatePositionsCache();
			if (component.texture)
			{
				auto& texturedBuffers = texturesToBuffers[*component.texture];
				texturedBuffers.positionsCache.insert(texturedBuffers.positionsCache.end(), positionsCache.begin(), positionsCache.end());
			}
			else
			{
				simpleBuffers.positionsCache.insert(simpleBuffers.positionsCache.end(), positionsCache.begin(), positionsCache.end());
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, simpleBuffers.positionBuffer);
		if (simpleBuffers.numOfAllocatedVertices < simpleBuffers.positionsCache.size())
		{
			glBufferData(GL_ARRAY_BUFFER, simpleBuffers.positionsCache.size() * sizeof(simpleBuffers.positionsCache.front()),
				simpleBuffers.positionsCache.data(), bufferDataUsage);
			simpleBuffers.numOfAllocatedVertices = simpleBuffers.positionsCache.size();
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, simpleBuffers.positionsCache.size() * sizeof(simpleBuffers.positionsCache.front()),
				simpleBuffers.positionsCache.data());
		}

		for (auto& [texture, texturedBuffers] : texturesToBuffers)
		{
			glBindBuffer(GL_ARRAY_BUFFER, texturedBuffers.positionBuffer);
			if (texturedBuffers.numOfAllocatedVertices < texturedBuffers.positionsCache.size())
			{
				glBufferData(GL_ARRAY_BUFFER, texturedBuffers.positionsCache.size() * sizeof(texturedBuffers.positionsCache.front()),
					texturedBuffers.positionsCache.data(), bufferDataUsage);
				texturedBuffers.numOfAllocatedVertices = texturedBuffers.positionsCache.size();
			}
			else
			{
				glBufferSubData(GL_ARRAY_BUFFER, 0, texturedBuffers.positionsCache.size() * sizeof(texturedBuffers.positionsCache.front()),
					texturedBuffers.positionsCache.data());
			}
		}
	}
}
