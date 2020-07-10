#pragma once

#include <chrono>

namespace Systems
{
	class Physics
	{
	public:
		Physics();

		void step();

	private:
		bool firstStep = true;

		std::chrono::high_resolution_clock::time_point start;
	};
}
