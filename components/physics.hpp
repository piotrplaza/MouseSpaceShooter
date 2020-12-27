#pragma once

#include <Box2D/Box2D.h>

#include <componentBase.hpp>

namespace Components
{
	struct Physics : ComponentBase
	{
		b2World world{ {0.0f, 0.0f} };

		float simulationTime = 0.0f;
		float frameTime = 0.0f;
	};
}
