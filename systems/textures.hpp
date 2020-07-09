#include <memory>
#include <string>
#include <unordered_map>

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
			int width;
			int height;
			int bitDepth;
		};

		void loadAndConfigureTexture(const Components::TextureDef& textureDef, Components::Texture& texture);

		std::unordered_map<std::string, TextureCache> pathsToTextureCaches;
	};
}
