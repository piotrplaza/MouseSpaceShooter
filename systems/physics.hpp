#pragma once

#include <chrono>
#include <optional>

namespace Systems
{
	class Physics
	{
	public:
		Physics();

		void postInit();
		void step();

#ifndef _DEBUG 
		std::chrono::high_resolution_clock::time_point prevFrameTime;
#endif
	};
}
