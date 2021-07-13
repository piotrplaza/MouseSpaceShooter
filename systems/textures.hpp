#include <memory>
#include <string>
#include <unordered_map>

#include <glm/vec2.hpp>

namespace Components
{
	struct TextureDef;
	struct Texture;
}

namespace Systems
{
	class Textures
	{
	public:
		Textures();

	private:
		struct TextureCache
		{
			std::unique_ptr<unsigned char[]> bytes;
			glm::ivec2 size = {0, 0};
			int bitDepth = 0;
		};

		void loadAndConfigureTexture(const Components::TextureDef& textureDef, Components::Texture& texture);
		void createLowResFramebuffersTextures() const;

		std::unordered_map<std::string, TextureCache> pathsToTexturesCache;
	};
}
