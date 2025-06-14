#pragma once

namespace Systems
{
	class Structures
	{
	public:
		Structures();

		void postInit();
		void step();

		void updateStaticBuffers();

	private:
		void updateDynamicBuffers();

		size_t loadedStaticWalls = 0;
		size_t loadedStaticGrapples = 0;
		size_t loadedStaticPolylines = 0;
	};
}
