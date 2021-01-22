#include "deferredActions.hpp"

namespace Systems
{
	DeferredActions::DeferredActions()
	{
	}

	void DeferredActions::addDeferredAction(std::function<bool()> deferredAction)
	{
		deferredActions.push_back(std::move(deferredAction));
	}

	void DeferredActions::step()
	{
		auto it = deferredActions.begin();
		while (it != deferredActions.end())
		{
			if ((*it)()) ++it;
			else it = deferredActions.erase(it);
		}
	}
}
