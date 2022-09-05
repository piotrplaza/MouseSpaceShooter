#pragma once

namespace Globals::CollisionBits
{
	constexpr unsigned short plane = 1 << 1;
	constexpr unsigned short missile = 1 << 2;
	constexpr unsigned short wall = 1 << 3;
	constexpr unsigned short shockwaveParticle = 1 << 4;

	constexpr unsigned short none = 0;
	constexpr unsigned short all = ~none;
}
