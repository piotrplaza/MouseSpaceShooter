#include "deferredActions.hpp"

#include <globals/components.hpp>

#include <components/deferredAction.hpp>
#include <components/physics.hpp>

namespace Systems
{
	void DeferredActions::step()
	{
		auto& deferredActions = Globals::Components().deferredActions();

		auto it = deferredActions.begin();
		while (it != deferredActions.end())
		{
			it->step();

			if (it->details.startTime < 0.0f)
				it->details.startTime = Globals::Components().physics().simulationDuration + it->delay;

			if (it->details.startTime <= Globals::Components().physics().simulationDuration)
			{
				if (it->deferredAction(Globals::Components().physics().simulationDuration - it->details.startTime, it->delay))
				{
					it->details.startTime += it->delay;
					++it;
				}
				else
					it = deferredActions.remove(it);
			}
			else
				++it;
		}
	}
}
