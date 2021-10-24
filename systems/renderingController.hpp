#pragma once

namespace Systems
{
	class RenderingController
	{
	public:
		RenderingController();

		void initializationFinalize() const;
		void render() const;

	private:
		void initGraphics() const;
	};
}
