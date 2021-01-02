#include "componentId.hpp"

#include <limits>
#include <cassert>

ComponentIdGenerator ComponentIdGenerator::instance_;

ComponentIdGenerator& ComponentIdGenerator::instance()
{
	return instance_;
}

ComponentId ComponentIdGenerator::acquire()
{
	if (counter < std::numeric_limits<ComponentId>::max())
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

ComponentId ComponentIdGenerator::current() const
{
	if (counter < std::numeric_limits<ComponentId>::max())
	{
		return counter;
	}
	else
	{
		assert(!releasedIds.empty());
		return releasedIds.back();
	}
}

void ComponentIdGenerator::release(ComponentId id)
{
	releasedIds.push_back(id);
}
