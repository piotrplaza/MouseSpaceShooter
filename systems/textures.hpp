#pragma once

#include <glm/vec2.hpp>

#include <commonTypes/idGenerator.hpp>

#include <ogl/oglProxy.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace Components
{
	struct Texture;
	struct RenderTexture;
}

struct TextureFile;
struct TextureData;

namespace Systems
{
	class Textures
	{
	public:
		struct TextureCache
		{
			std::unique_ptr<float[]> data;
			glm::ivec2 size = { 0, 0 };
			int numOfChannels = 0;
		};

		Textures();

		void postInit();
		void step();

		void updateStaticTextures();
		void updateStaticRenderTextures();

		const TextureCache& loadFile(const TextureFile& file);
		const TextureCache& textureDataFromFile(TextureData& textureData);

	private:
		void updateDynamicTextures();
		void updateTexture(Components::Texture& texture);
		void deleteTexture(Components::Texture& texture);
		void loadAndConfigureTexture(Components::Texture& texture);
		void createAndConfigureStandardRenderTextures();
		void updateDynamicRenderTextures();
		void updateRenderTexture(Components::RenderTexture& renderTexture);
		void deleteRenderTexture(Components::RenderTexture& renderTexture);
		void configureRenderTexture(Components::RenderTexture& renderTexture);

		unsigned staticTexturesOffset = 0;
		unsigned staticRenderTexturesOffset = 0;
		std::unordered_map<std::string, TextureCache> keysToTexturesCache;
		std::vector<float> operationalBuffer;
	};
}
