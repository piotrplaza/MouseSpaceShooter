#include "cleaner.hpp"

#include <components/sound.hpp>
#include <components/plane.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/polyline.hpp>
#include <components/decoration.hpp>
#include <components/missile.hpp>
#include <components/collisionFilter.hpp>
#include <components/collisionHandler.hpp>
#include <components/shockwave.hpp>
#include <components/light.hpp>
#include <components/functor.hpp>
#include <components/deferredAction.hpp>

#include <globals/components.hpp>

namespace Systems
{
	Cleaner::Cleaner() = default;

	void Cleaner::step() const
	{
		Globals::Components().sounds().removeOutdated();
		Globals::Components().planes().removeOutdated();
		Globals::Components().dynamicWalls().removeOutdated();
		Globals::Components().grapples().removeOutdated();
		Globals::Components().dynamicPolylines().removeOutdated();
		Globals::Components().dynamicDecorations().removeOutdated();
		Globals::Components().missiles().removeOutdated();
		Globals::Components().collisionFilters().removeOutdated();
		Globals::Components().beginCollisionHandlers().removeOutdated();
		Globals::Components().endCollisionHandlers().removeOutdated();
		Globals::Components().shockwaves().removeOutdated();
		Globals::Components().lights().removeOutdated();
		Globals::Components().stepSetups().removeOutdated();
		Globals::Components().stepTeardowns().removeOutdated();
		Globals::Components().deferredActions().removeOutdated();
	}
}
