#include "componentBase.hpp"

#include <vector>
#include <limits>
#include <cassert>

static class ComponentIdGenerator
{
public:
	ComponentBase::Id acquire()
	{
		if (counter < std::numeric_limits<unsigned long>::max())
		{
			return counter++;
		}
		else
		{
			assert(!releasedIds.empty());
			const auto id = releasedIds.back();
			releasedIds.pop_back();
			return id;
		}
	}

	void release(ComponentBase::Id id)
	{
		releasedIds.push_back(id);
	}

private:
	ComponentBase::Id counter = 0;
	std::vector<ComponentBase::Id> releasedIds;
} componentIdGenerator;

void ComponentBase::ReleaseId(Id id)
{
	componentIdGenerator.release(id);
}

ComponentBase::ComponentBase() :
	id(componentIdGenerator.acquire())
{
}
