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
		DynamicComponentsBase::removeAllOutdated();
	}
}
