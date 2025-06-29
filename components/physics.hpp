#pragma once

#include "_componentBase.hpp"

#include <Box2D/Box2D.h>

#include <glm/vec2.hpp>

#include <memory>
#include <chrono>

namespace Components
{
	struct Physics : ComponentBase
	{
		std::unique_ptr<b2World> world;

		float simulationDuration = 0.0f;
		float frameDuration = 0.0f;
		unsigned long frameCount = 0;
		float gameSpeed = 1.0f;

		int forceRefreshRateOrTimeBasedStep = 0;
		float minFPS = 58.0f;
		int velocityIterationsPerStep = 3;
		int positionIterationsPerStep = 8;

		bool paused = false;

		std::chrono::high_resolution_clock::time_point prevFrameTime;
	};
}
