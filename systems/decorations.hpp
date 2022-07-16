#pragma once

namespace Systems
{
	class Decorations
	{
	public:
		Decorations();

		void postInit();
		void step();

		void updateStaticBuffers();

	private:
		void updateDynamicBuffers();

		size_t loadedStaticDecorations = 0;
	};
}
