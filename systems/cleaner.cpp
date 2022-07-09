#include "cleaner.hpp"

#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/decoration.hpp>
#include <components/missile.hpp>
#include <components/collisionHandler.hpp>
#include <components/shockwave.hpp>
#include <components/light.hpp>
#include <components/functor.hpp>

#include <components/_componentBase.hpp>
#include <components/_componentId.hpp>

#include <globals/components.hpp>

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
		RemoveOutdatedComponents(Globals::Components().dynamicWalls());
		RemoveOutdatedComponents(Globals::Components().grapples());
		RemoveOutdatedComponents(Globals::Components().dynamicDecorations());
		RemoveOutdatedComponents(Globals::Components().missiles());
		RemoveOutdatedComponents(Globals::Components().beginCollisionHandlers());
		RemoveOutdatedComponents(Globals::Components().endCollisionHandlers());
		RemoveOutdatedComponents(Globals::Components().shockwaves());
		RemoveOutdatedComponents(Globals::Components().lights());
		RemoveOutdatedComponents(Globals::Components().frameSetups());
		RemoveOutdatedComponents(Globals::Components().frameTeardowns());
	}
}
