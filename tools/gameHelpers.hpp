#pragma once

#include <glm/vec2.hpp>

namespace Tools
{
	struct PlayerPlaneHandler
	{
		size_t playerId = 0;
		size_t backThrustsBackgroundDecorationIds[2] = {};
	};

	struct MissileHandler
	{
		size_t missileId = 0;
		size_t backThrustTemporaryBackgroundDecoration = 0;
	};

	PlayerPlaneHandler CreatePlayerPlane(unsigned planeTexture, unsigned flameAnimationTexture);
	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 initialVelocity, unsigned missileTexture, unsigned flameAnimationTexture);
}
