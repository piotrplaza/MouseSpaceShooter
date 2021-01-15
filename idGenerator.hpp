#pragma once

#include <vector>
#include <limits>
#include <cassert>

template <typename Id, Id firstId = 0>
class IdGenerator
{
public:
	static IdGenerator& instance()
	{
		return instance_;
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
			const auto id = releasedIds.back();
			releasedIds.pop_back();
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
			return releasedIds.back();
		}
	}

	void release(Id id)
	{
		releasedIds.push_back(id);
	}


private:
	IdGenerator() = default;

	static IdGenerator<Id, firstId> instance_;

	Id counter = firstId;
	std::vector<Id> releasedIds;
};
