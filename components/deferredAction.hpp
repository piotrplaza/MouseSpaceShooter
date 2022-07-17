#pragma once

#include "_componentBase.hpp"

#include <functional>

namespace Components
{
	struct DeferredAction : ComponentBase
	{
		DeferredAction() = default;

		DeferredAction(std::function<bool(float duration)> deferredAction, float delay = 0.0f) :
			deferredAction(std::move(deferredAction)),
			delay(delay)
		{
		}

		std::function<bool(float duration)> deferredAction;
		float delay = 0.0f;

		struct
		{
			float startTime = -1.0f;
		} details;
	};
}
