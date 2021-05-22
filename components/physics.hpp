#pragma once

#include <Box2D/Box2D.h>

#include <componentBase.hpp>

namespace Components
{
	struct Physics : ComponentBase
	{
		b2World world{ {0.0f, 0.0f} };

		float simulationDuration = 0.0f;
		float frameDuration = 0.0f;
	};
}
