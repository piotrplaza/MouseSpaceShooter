#pragma once

#include "_componentBase.hpp"

#include <commonTypes/fTypes.hpp>

namespace Components
{
	struct DeferredAction : ComponentBase
	{
		DeferredAction(std::function<bool(float duration, float& delay)> deferredAction = nullptr, float delay = 0.0f) :
			deferredAction(std::move(deferredAction)),
			delay(delay)
		{
		}

		DeferredAction(std::function<bool(float duration)> deferredAction = nullptr, float delay = 0.0f) :
			deferredAction([=](float duration, auto) { return deferredAction(duration); }),
			delay(delay)
		{
		}

		DeferredAction(FBool deferredAction, float delay = 0.0f) :
			deferredAction([=](auto, auto) { return deferredAction(); }),
			delay(delay)
		{
		}

		std::function<bool(float duration, float& delay)> deferredAction;
		float delay = 0.0f;

		struct
		{
			float startTime = -1.0f;
		} details;
	};
}
