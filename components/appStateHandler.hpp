#pragma once

#include "_componentBase.hpp"

#include "keyboard.hpp"


namespace Components
{
	struct AppStateHandler : ComponentBase
	{
		std::function<bool(bool prevPauseState)> pauseF = [](bool prevPauseState) { return !prevPauseState; };
		std::function<bool()> exitF = []() { return false; };
	};
}
