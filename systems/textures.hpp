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

		const TextureCache& loadFile(const TextureFile& file);
		const TextureCache& textureDataFromFile(TextureData& textureData);

	private:
		void loadAndConfigureTexture(Components::Texture& texture);
		void createTextureFramebuffers();
		void updateStaticTextures();
		void updateDynamicTextures();
		void deleteTexture(Components::Texture& texture);

		unsigned staticTexturesOffset = 0;
		std::unordered_map<std::string, TextureCache> keysToTexturesCache;
		std::vector<float> operationalBuffer;
	};
}
