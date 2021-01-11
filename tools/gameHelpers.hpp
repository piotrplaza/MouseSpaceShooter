#pragma once

#include <glm/vec2.hpp>

#include <componentId.hpp>

namespace Tools
{
	struct PlayerPlaneHandler
	{
		size_t playerId = 0;
		size_t backThrustsIds[2] = {};
	};

	struct MissileHandler
	{
		ComponentId missileId = 0;
		ComponentId backThrustId = 0;
	};

	PlayerPlaneHandler CreatePlayerPlane(unsigned planeTexture, unsigned flameAnimationTexture);
	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 initialVelocity, unsigned missileTexture, unsigned flameAnimationTexture);
}
