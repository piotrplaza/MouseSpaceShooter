#pragma once

#include <list>
#include <functional>

namespace Systems
{
	class DeferredActions
	{
	public:
		DeferredActions();

		void addDeferredAction(std::function<bool()> deferredAction);
		void step();

	private:
		std::list<std::function<bool()>> deferredActions;
	};
}
