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
		void pause();
		void resume();

	private:
		float pauseDuration = 0.0f;

#ifndef _DEBUG 
		std::chrono::high_resolution_clock::time_point startTime;
#endif
		std::optional<std::chrono::high_resolution_clock::time_point> pauseTime;
	};
}
