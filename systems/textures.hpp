#include <memory>
#include <string>
#include <unordered_map>

#include <glm/vec2.hpp>

namespace Components
{
	struct Texture;
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
			std::unique_ptr<unsigned char[]> bytes;
			glm::ivec2 size = {0, 0};
			int bitDepth = 0;
		};

		void loadAndConfigureTexture(Components::Texture& texture);
		void createTextureFramebuffers() const;

		std::unordered_map<std::string, TextureCache> pathsToTexturesCache;
	};
}
