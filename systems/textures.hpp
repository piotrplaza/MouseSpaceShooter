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

namespace Systems
{
	class Textures
	{
	public:
		Textures();

		void postInit();
		void step();

	private:
		struct TextureCache
		{
			std::unique_ptr<float[]> data;
			glm::ivec2 size = {0, 0};
			int numOfChannels = 0;
			bool premultipliedAlpha = false;
			bool darkToTransparent = false;
		};

		void loadAndConfigureTexture(Components::Texture& texture);
		void createTextureFramebuffers();
		void updateDynamicTextures();
		void deleteTexture(Components::Texture& texture);

		IdGenerator<unsigned, GL_TEXTURE0, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS> textureUnits;
		unsigned staticTexturesOffset = 0;
		std::unordered_map<std::string, TextureCache> pathsToTexturesCache;
	};
}
