#pragma once

#include "componentBase.hpp"

#include <Box2D/Box2D.h>

#include <glm/vec2.hpp>

#include <memory>

namespace Components
{
	struct Physics : ComponentBase
	{
		using ComponentBase::ComponentBase;

		Physics()
		{
			world = std::make_unique<b2World>(b2Vec2(0.0f, 0.0f));
		}

		std::unique_ptr<b2World> world;

		float simulationDuration = 0.0f;
		float frameDuration = 0.0f;
		float gameSpeed = 1.0f;

		bool paused = false;
	};
}
