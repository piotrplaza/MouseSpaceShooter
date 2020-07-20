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
	inline void UpdateTransformedPositionsBuffers(const std::vector<Component>& components, Buffers& simpleBuffers,
		std::unordered_map<unsigned, Buffers>& texturesToBuffers, std::vector<Buffers>& customSimpleBuffers,
		std::vector<Buffers>& customTexturedBuffers, GLenum bufferDataUsage)
	{
		simpleBuffers.positionsCache.clear();
		for (auto& [texture, buffers] : texturesToBuffers) buffers.positionsCache.clear();
		for (auto& buffers: customSimpleBuffers) buffers.positionsCache.clear();
		for (auto& buffers: customTexturedBuffers) buffers.positionsCache.clear();

		auto customSimpleBuffersIt = customSimpleBuffers.begin();
		auto customTexturedBuffersIt = customTexturedBuffers.begin();

		for (auto& component : components)
		{
			auto& buffers = [&]() -> auto&
			{
				if (component.texture)
				{
					if (component.renderingSetup)
					{
						auto& buffers = customTexturedBuffersIt == customTexturedBuffers.end()
							? customTexturedBuffers.emplace_back()
							: *customTexturedBuffersIt++;
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
						auto& buffers = customSimpleBuffersIt == customSimpleBuffers.end()
							? customSimpleBuffers.emplace_back()
							: *customSimpleBuffersIt++;
						buffers.renderingSetup = component.renderingSetup;
						return buffers;
					}
					else
					{
						return simpleBuffers;
					}
				}
			}();

			const auto& positionsCache = component.getTransformedPositionsCache();
			buffers.positionsCache.insert(buffers.positionsCache.end(),
				positionsCache.begin(), positionsCache.end());
		}

		Detail::AllocateOrUpdatePositionsData(simpleBuffers, bufferDataUsage);
		for (auto& [texture, buffers] : texturesToBuffers) Detail::AllocateOrUpdatePositionsData(buffers, bufferDataUsage);
		for (auto& buffers : customSimpleBuffers) Detail::AllocateOrUpdatePositionsData(buffers, bufferDataUsage);
		for (auto& buffers : customTexturedBuffers) Detail::AllocateOrUpdatePositionsData(buffers, bufferDataUsage);
	}

	template <typename Component, typename Buffers>
	inline void UpdatePositionsBuffers(const std::vector<Component>& components, std::vector<Buffers>& simpleBuffers,
		std::vector<Buffers>& texturedBuffers, std::vector<Buffers>& customShadersBuffers, GLenum bufferDataUsage)
	{
		auto simpleBuffersIt = simpleBuffers.begin();
		auto texturedBuffersIt = texturedBuffers.begin();
		auto customBuffersIt = customShadersBuffers.begin();

		for (auto& component : components)
		{
			auto& buffers = [&]() -> auto&
			{
				if (component.customShadersProgram)
				{
					auto& buffers = customBuffersIt == customShadersBuffers.end()
						? customShadersBuffers.emplace_back()
						: *customBuffersIt++;
					return buffers;
				}
				else if (component.texture)
				{
					auto& buffers = texturedBuffersIt == texturedBuffers.end()
						? texturedBuffers.emplace_back()
						: *texturedBuffersIt++;
					return buffers;
				}
				else
				{
					auto& buffers = simpleBuffersIt == simpleBuffers.end()
						? simpleBuffers.emplace_back()
						: *simpleBuffersIt++;
					return buffers;
				}
			}();

			const auto& positionsCache = component.getPositionsCache();

			buffers.renderingSetup = component.renderingSetup;
			buffers.texture = component.texture;
			buffers.customShadersProgram = component.customShadersProgram;
			buffers.positionsCache.clear();
			buffers.positionsCache.insert(buffers.positionsCache.end(), positionsCache.begin(), positionsCache.end());
			Detail::AllocateOrUpdatePositionsData(buffers, bufferDataUsage);
		}
	}

	template <typename Component, typename Buffers>
	inline void UpdateTexCoordBuffers(const std::vector<Component>& components,
		std::unordered_map<unsigned, Buffers>& texturesToBuffers, std::vector<Buffers>& customTexturedBuffers,
		GLenum bufferDataUsage)
	{
		for (auto& [texture, buffers] : texturesToBuffers) buffers.texCoordCache.clear();
		for (auto& buffers : customTexturedBuffers) buffers.texCoordCache.clear();

		auto customTexturedBuffersIt = customTexturedBuffers.begin();

		for (auto& component : components)
		{
			if (component.texture)
			{
				const auto& positionsCache = component.getPositionsCache();
				auto& buffers = [&]() -> auto&
				{
					if (component.renderingSetup)
					{
						return customTexturedBuffersIt == customTexturedBuffers.end()
							? customTexturedBuffers.emplace_back()
							: *customTexturedBuffersIt++;
					}
					else
					{
						return texturesToBuffers[*component.texture];
					}
				}();

				buffers.texture = component.texture;
				if (!buffers.texCoordBuffer) buffers.createTexCoordBuffer();

				buffers.texCoordCache.insert(buffers.texCoordCache.end(), positionsCache.begin(), positionsCache.end());
			}
		}

		for (auto& [texture, buffers] : texturesToBuffers) Detail::AllocateOrUpdateTexCoordData(buffers, bufferDataUsage);
		for (auto& buffers : customTexturedBuffers) Detail::AllocateOrUpdateTexCoordData(buffers, bufferDataUsage);
	}

	template <typename Component, typename Buffers>
	inline void UpdateTexCoordBuffers(const std::vector<Component>& components,
		std::vector<Buffers>& texturedBuffers, std::vector<Buffers>& customShadersTexturedBuffers, GLenum bufferDataUsage)
	{
		auto texturedBuffersIt = texturedBuffers.begin();
		auto customShadersTexturedBuffersIt = customShadersTexturedBuffers.begin();

		for (auto& component : components)
		{
			if (component.texture)
			{
				auto& relevantBuffers = component.customShadersProgram
					? customShadersTexturedBuffers
					: texturedBuffers;
				auto& relevantBuffersIt = component.customShadersProgram
					? customShadersTexturedBuffersIt
					: texturedBuffersIt;

				const auto& positionsCache = component.getPositionsCache();
				auto& buffers = relevantBuffersIt == relevantBuffers.end()
					? relevantBuffers.emplace_back()
					: *relevantBuffersIt++;

				buffers.texture = component.texture;
				if (!buffers.texCoordBuffer) buffers.createTexCoordBuffer();

				buffers.texCoordCache.clear();
				buffers.texCoordCache.insert(buffers.texCoordCache.end(), positionsCache.begin(), positionsCache.end());
				Detail::AllocateOrUpdateTexCoordData(buffers, bufferDataUsage);
			}
		}
	}
}
