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
			ComponentIdGenerator::instance().release(it->first);
			it = components.erase(it);
		}
		else
			++it;
	}
}

namespace Systems
{
	Cleaner::Cleaner() = default;

	void Cleaner::step()
	{
		using namespace Globals::Components;

		RemoveOutdatedComponents(temporaryBackgroundDecorations);
		RemoveOutdatedComponents(temporaryFarMidgroundDecorations);
		RemoveOutdatedComponents(temporaryNearMidgroundDecorations);
		RemoveOutdatedComponents(temporaryForegroundDecorations);
		RemoveOutdatedComponents(missiles);
		RemoveOutdatedComponents(shockwaves);
	}
}
