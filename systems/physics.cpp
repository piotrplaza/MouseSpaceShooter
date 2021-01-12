#include "physics.hpp"

#include <globals.hpp>

#include <components/physics.hpp>

namespace
{
	constexpr float debugFrameTime = 1.0f / 160;
}

namespace Systems
{
	Physics::Physics() = default;

	void Physics::step()
	{
		using namespace Globals::Components;

		if (firstStep)
		{
#ifndef _DEBUG 
			start = std::chrono::high_resolution_clock::now();
#endif
			firstStep = false;
		}

#ifndef _DEBUG 
		const auto simulationTime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();
		physics.frameTime = simulationTime - physics.simulationTime;
		physics.simulationTime = simulationTime;
#else
		physics.frameTime = debugFrameTime;
		physics.simulationTime += physics.frameTime;
#endif

		physics.world.Step(physics.frameTime, 3, 8);
	}
}
