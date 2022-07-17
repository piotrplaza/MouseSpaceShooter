#pragma once

#include "_componentBase.hpp"

#include <array>

namespace Components
{
	struct KeyboardState : ComponentBase
	{
		std::array<bool, 256> pressed;
		std::array<bool, 256> released;
		std::array<bool, 256> pressing;
	};
}
