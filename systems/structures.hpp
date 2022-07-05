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
	};
}
