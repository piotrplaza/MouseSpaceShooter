#pragma once

namespace Tools
{
	struct PlayerPlaneHandler
	{
		size_t playerId = 0;
		size_t backThrustsBackgroundDecorationIds[2] = {};
	};

	PlayerPlaneHandler CreatePlayerPlane(unsigned planeTexture, unsigned flameTexture);
}
