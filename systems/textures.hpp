#pragma once

#include <glm/vec2.hpp>

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

	private:
		struct TextureCache
		{
			std::unique_ptr<float[]> channels;
			glm::ivec2 size = {0, 0};
			int bitDepth = 0;
			bool premultipliedAlpha = false;
			bool darkToTransparent = false;
		};

		void loadAndConfigureTexture(Components::Texture& texture);
		void createTextureFramebuffers() const;

		unsigned customTexturesOffset = 0;
		std::unordered_map<std::string, TextureCache> pathsToTexturesCache;
	};
}
