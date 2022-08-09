#pragma once

#include <optional>

namespace Systems
{
	class Physics
	{
	public:
		Physics();

		void postInit();
		void step();
	};
}
