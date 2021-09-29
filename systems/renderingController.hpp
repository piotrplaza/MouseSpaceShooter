#pragma once

namespace Systems
{
	class RenderingController
	{
	public:
		RenderingController();

		void render();

	private:
		void initGraphics();
	};
}
