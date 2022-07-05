#pragma once

namespace Systems
{
	class RenderingController
	{
	public:
		RenderingController();

		void postInit() const;
		void render() const;

	private:
		void basicRender(size_t layer) const;
		void texturedRender(size_t layer) const;
		void customShadersRender(size_t layer) const;


	};
}
