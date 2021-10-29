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
		void initGraphics() const;
	};
}
