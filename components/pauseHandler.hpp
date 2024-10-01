#pragma once

#include "_componentBase.hpp"

namespace Components
{
	struct PauseHandler : ComponentBase
	{
		std::function<bool(bool prevPauseState)> handler = [](bool prevPauseState) { return !prevPauseState; };
	};
}
