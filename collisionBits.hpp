#pragma once

namespace CollisionBits
{
	constexpr unsigned short playerBit = 1 << 1;
	constexpr unsigned short missileBit = 1 << 2;
	constexpr unsigned short wallBit = 1 << 3;

	constexpr unsigned short none = 0;
	constexpr unsigned short all = ~none;
}
