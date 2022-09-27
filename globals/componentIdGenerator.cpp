#include "componentIdGenerator.hpp"

#include <memory>

namespace Globals
{
	static std::unique_ptr<IdGenerator<ComponentId, 1>> componentIdGenerator;

	void InitializeComponentIdGenerator()
	{
		componentIdGenerator = std::make_unique<IdGenerator<ComponentId, 1>>();
	}

	IdGenerator<ComponentId, 1>& ComponentIdGenerator()
	{
		return *componentIdGenerator;
	}
}
