#pragma once

#include "_componentBase.hpp"

#include <tools/b2Helpers.hpp>

#include <functional>

class b2Fixture;

namespace Components
{
	struct CollisionFilter : ComponentBase
	{
		enum class Result { False, True, Fallback };

		CollisionFilter(b2Fixture& fixtureA, b2Fixture& fixtureB, std::function<bool(b2Fixture&, b2Fixture&)> handler):
			rawFilter([&fixtureA, &fixtureB, relevantSortedFixtures = Tools::Sort(&fixtureA, &fixtureB), handler = std::move(handler)](b2Fixture& fixtureA_, b2Fixture& fixtureB_)
			{
				if (relevantSortedFixtures != Tools::Sort(&fixtureA_, &fixtureB_))
					return Result::Fallback;

				return (Result)handler(fixtureA, fixtureB);
			})
		{
		}

		CollisionFilter(unsigned short categoryBits, unsigned short maskBits, std::function<bool(b2Fixture&, b2Fixture&)> handler):
			rawFilter([categoryBits, maskBits, handler = std::move(handler)](b2Fixture& fixtureA, b2Fixture& fixtureB)
			{
				if (categoryBits == fixtureA.GetFilterData().categoryBits && fixtureB.GetFilterData().categoryBits & maskBits)
					return (Result)handler(fixtureA, fixtureB);
				if (categoryBits == fixtureB.GetFilterData().categoryBits && fixtureA.GetFilterData().categoryBits & maskBits)
					return (Result)handler(fixtureB, fixtureA);

				return Result::Fallback;
			})
		{
		}

		const std::function<Result(b2Fixture&, b2Fixture&)> rawFilter;
	};
}
