#pragma once

namespace Systems
{
	class Temporaries
	{
	public:
		Temporaries();

		void step();

	private:
		void updateDynamicBuffers();
	};
}
