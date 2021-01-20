#pragma once

#include <functional>

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
		MissileHandler();
		MissileHandler(ComponentId missileId, ComponentId backThrustId);
		~MissileHandler();
		MissileHandler(MissileHandler&& other) noexcept;
		MissileHandler& operator=(MissileHandler&& other) noexcept;

		ComponentId missileId = 0;
		ComponentId backThrustId = 0;

	private:
		bool valid = true;
	};

	PlayerPlaneHandler CreatePlayerPlane(unsigned planeTexture, unsigned flameAnimationTexture);
	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 initialVelocity, unsigned missileTexture, unsigned flameAnimationTexture);
}
