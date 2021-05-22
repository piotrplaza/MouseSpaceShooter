#pragma once

#include <chrono>
#include <optional>

namespace Systems
{
	class Physics
	{
	public:
		Physics();

		void step();
		void pause();
		void resume();

	private:
		bool firstStep = true;
		float pauseDuration = 0.0f;

		std::chrono::high_resolution_clock::time_point startTime;
		std::optional<std::chrono::high_resolution_clock::time_point> pauseTime;
	};
}
