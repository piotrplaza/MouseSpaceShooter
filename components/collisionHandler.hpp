#pragma once

#include "_componentBase.hpp"

#include <tools/b2Helpers.hpp>

#include <functional>

class b2Fixture;

namespace Components
{
	struct CollisionHandler : ComponentBase
	{
		CollisionHandler(b2Fixture& fixtureA, b2Fixture& fixtureB, std::function<void(b2Fixture&, b2Fixture&)> handler):
			rawHandler([&fixtureA, &fixtureB, relevantSortedFixtures = Tools::Sort(&fixtureA, &fixtureB), handler = std::move(handler)](b2Fixture& fixtureA_, b2Fixture& fixtureB_)
			{
				if (relevantSortedFixtures == Tools::Sort(&fixtureA_, &fixtureB_))
					handler(fixtureA, fixtureB);
			})
		{
		}

		CollisionHandler(unsigned short categoryBits, unsigned short maskBits, std::function<void(b2Fixture&, b2Fixture&)> handler):
			rawHandler([categoryBits, maskBits, handler = std::move(handler)](b2Fixture& fixtureA, b2Fixture& fixtureB)
			{
				if (categoryBits == fixtureA.GetFilterData().categoryBits && fixtureB.GetFilterData().categoryBits & maskBits)
					handler(fixtureA, fixtureB);
				else if (categoryBits == fixtureB.GetFilterData().categoryBits && fixtureA.GetFilterData().categoryBits & maskBits)
					handler(fixtureB, fixtureA);
			})
		{
		}

		const std::function<void(b2Fixture&, b2Fixture&)> rawHandler;
	};
}
