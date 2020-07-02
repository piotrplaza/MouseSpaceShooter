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

	private:
		void loadAndConfigureTexture(Components::Texture& texture) const;
	};
}
