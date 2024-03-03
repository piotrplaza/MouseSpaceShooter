#pragma once

#include <deque>
#include <limits>
#include <cassert>

template <typename IdType, IdType firstId = 0, IdType max = std::numeric_limits<IdType>::max()>
class IdGenerator
{
public:
	IdGenerator()
	{
	}

	IdType acquire()
	{
		if (counter < max)
			return counter++;

		assert(!releasedIds.empty());
		const auto id = releasedIds.front();
		releasedIds.pop_front();
		return id;
	}

	IdType current() const
	{
		if (counter < max)
			return counter;

		assert(!releasedIds.empty());
		return releasedIds.front();
	}

	void release(IdType id)
	{
		releasedIds.push_back(id);
	}

private:
	IdType counter = firstId;
	std::deque<IdType> releasedIds;
};
