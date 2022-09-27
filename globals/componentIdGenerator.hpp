#pragma once

#include <commonTypes/idGenerator.hpp>
#include <commonTypes/componentId.hpp>

namespace Globals
{
	void InitializeComponentIdGenerator();
	IdGenerator<ComponentId, 1>& ComponentIdGenerator();
}
