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
		void loadAndConfigureTexture(const Components::TextureDef& textureDef, Components::Texture& texture) const;
	};
}
