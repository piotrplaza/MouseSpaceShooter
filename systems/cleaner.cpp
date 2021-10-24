#include "cleaner.hpp"

#include <globals.hpp>
#include <componentBase.hpp>
#include <componentId.hpp>

#include <components/decoration.hpp>
#include <components/missile.hpp>
#include <components/shockwave.hpp>

template <typename ComponentsMap>
inline void RemoveOutdatedComponents(ComponentsMap& components)
{
	auto it = components.begin();
	while (it != components.end())
	{
		if (it->second.state == ComponentState::Outdated)
		{
			ComponentIdGenerator().release(it->first);
			it = components.erase(it);
		}
		else
			++it;
	}
}

namespace Systems
{
	Cleaner::Cleaner() = default;

	void Cleaner::step() const
	{
		RemoveOutdatedComponents(Globals::Components().temporaryBackgroundDecorations());
		RemoveOutdatedComponents(Globals::Components().temporaryFarMidgroundDecorations());
		RemoveOutdatedComponents(Globals::Components().temporaryNearMidgroundDecorations());
		RemoveOutdatedComponents(Globals::Components().temporaryForegroundDecorations());
		RemoveOutdatedComponents(Globals::Components().missiles());
		RemoveOutdatedComponents(Globals::Components().shockwaves());
	}
}
