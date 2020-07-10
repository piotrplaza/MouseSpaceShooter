#include <vector>
#include <unordered_map>

#include <GL/glew.h>

namespace Tools
{
	template <typename Component, typename Buffers>
	void UpdateSimpleAndTexturesBuffers(std::vector<Component>& components, Buffers& simpleBuffers,
		std::unordered_map<unsigned, Buffers>& texturesToBuffers, GLenum bufferDataUsage)
	{
		simpleBuffers.verticesCache.clear();
		for (auto& [texture, buffers] : texturesToBuffers)
		{
			buffers.verticesCache.clear();
		}

		for (auto& component : components)
		{
			const auto verticesCache = component.generateVerticesCache();
			if (component.texture)
			{
				auto& texturedBuffers = texturesToBuffers[*component.texture];
				texturedBuffers.verticesCache.insert(texturedBuffers.verticesCache.end(), verticesCache.begin(), verticesCache.end());
			}
			else
			{
				simpleBuffers.verticesCache.insert(simpleBuffers.verticesCache.end(), verticesCache.begin(), verticesCache.end());
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, simpleBuffers.vertexBuffer);
		if (simpleBuffers.vertexBufferAllocation < simpleBuffers.verticesCache.size())
		{
			glBufferData(GL_ARRAY_BUFFER, simpleBuffers.verticesCache.size() * sizeof(simpleBuffers.verticesCache.front()),
				simpleBuffers.verticesCache.data(), bufferDataUsage);
			simpleBuffers.vertexBufferAllocation = simpleBuffers.verticesCache.size();
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, simpleBuffers.verticesCache.size() * sizeof(simpleBuffers.verticesCache.front()),
				simpleBuffers.verticesCache.data());
		}

		for (auto& [texture, texturedBuffers] : texturesToBuffers)
		{
			glBindBuffer(GL_ARRAY_BUFFER, texturedBuffers.vertexBuffer);
			if (texturedBuffers.vertexBufferAllocation < texturedBuffers.verticesCache.size())
			{
				glBufferData(GL_ARRAY_BUFFER, texturedBuffers.verticesCache.size() * sizeof(texturedBuffers.verticesCache.front()),
					texturedBuffers.verticesCache.data(), bufferDataUsage);
				texturedBuffers.vertexBufferAllocation = texturedBuffers.verticesCache.size();
			}
			else
			{
				glBufferSubData(GL_ARRAY_BUFFER, 0, texturedBuffers.verticesCache.size() * sizeof(texturedBuffers.verticesCache.front()),
					texturedBuffers.verticesCache.data());
			}
		}
	}
}
