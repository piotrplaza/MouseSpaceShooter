#pragma once

#include <deque>
#include <limits>
#include <cassert>

template <typename Id, Id firstId = 0>
class IdGenerator
{
public:
	IdGenerator()
	{
	}

	Id acquire()
	{
		if (counter < std::numeric_limits<Id>::max())
		{
			return counter++;
		}
		else
		{
			assert(!releasedIds.empty());
			const auto id = releasedIds.front();
			releasedIds.pop_front();
			return id;
		}
	}

	Id current() const
	{
		if (counter < std::numeric_limits<Id>::max())
		{
			return counter;
		}
		else
		{
			assert(!releasedIds.empty());
			return releasedIds.front();
		}
	}

	void release(Id id)
	{
		releasedIds.push_back(id);
	}

private:
	Id counter = firstId;
	std::deque<Id> releasedIds;
};
