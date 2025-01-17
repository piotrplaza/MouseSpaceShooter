#include "cleaner.hpp"

#include <globals/components.hpp>

namespace Systems
{
	Cleaner::Cleaner() = default;

	void Cleaner::step() const
	{
		DynamicComponentsBase::CleanupAllEnding();
	}
}
