#pragma once

#include "_componentBase.hpp"

#include <array>

namespace Components
{
	struct Keyboard : ComponentBase
	{
		std::array<bool, 256> pressed{};
		std::array<bool, 256> released{};
		std::array<bool, 256> pressing{};
	};
}
