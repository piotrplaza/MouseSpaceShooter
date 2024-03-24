#pragma once

#include <optional>

namespace Systems
{
	class Physics
	{
	public:
		Physics();
		~Physics();

		void postInit();
		void step(bool paused = false);
	};
}
