#include "cleaner.hpp"

#include <components/plane.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/decoration.hpp>
#include <components/missile.hpp>
#include <components/collisionHandler.hpp>
#include <components/shockwave.hpp>
#include <components/light.hpp>
#include <components/functor.hpp>

#include <globals/components.hpp>

namespace Systems
{
	Cleaner::Cleaner() = default;

	void Cleaner::step() const
	{
		Globals::Components().planes().removeOutdated();
		Globals::Components().dynamicWalls().removeOutdated();
		Globals::Components().grapples().removeOutdated();
		Globals::Components().dynamicDecorations().removeOutdated();
		Globals::Components().missiles().removeOutdated();
		Globals::Components().beginCollisionHandlers().removeOutdated();
		Globals::Components().endCollisionHandlers().removeOutdated();
		Globals::Components().shockwaves().removeOutdated();
		Globals::Components().lights().removeOutdated();
		Globals::Components().frameSetups().removeOutdated();
		Globals::Components().frameTeardowns().removeOutdated();
	}
}
