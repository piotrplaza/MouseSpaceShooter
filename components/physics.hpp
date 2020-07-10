#pragma once

#include <Box2D/Box2D.h>

namespace Components
{
	struct Physics
	{
		b2World world{ {0.0f, 0.0f} };

		float simulationTime = 0.0f;
		float frameTime = 0.0f;
		float targetFrameTimeFactor = 0.0f;
	};
}
