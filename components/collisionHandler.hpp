#pragma once

#include "_componentBase.hpp"

#include <tools/b2Helpers.hpp>

#include <functional>

class b2Fixture;

namespace Components
{
	struct CollisionHandler : ComponentBase
	{
		using ComponentBase::ComponentBase;

		CollisionHandler(const b2Fixture& fixtureA, const b2Fixture& fixtureB, std::function<void(b2Fixture&, b2Fixture&)> handler):
			handler([relevantSortedFixtures = Tools::Sort(&fixtureA, &fixtureB), handler = std::move(handler)](b2Fixture& fixtureA, b2Fixture& fixtureB)
			{
				if (relevantSortedFixtures == Tools::Sort(&fixtureA, &fixtureB))
					handler(fixtureA, fixtureB);
			})
		{
		}

		CollisionHandler(unsigned short categoryBits, unsigned short maskBits, std::function<void(b2Fixture&, b2Fixture&)> handler):
			handler([categoryBits, maskBits, handler = std::move(handler)](b2Fixture& fixtureA, b2Fixture& fixtureB)
			{
				if ((categoryBits & fixtureA.GetFilterData().categoryBits && fixtureB.GetFilterData().categoryBits & maskBits) || 
					(categoryBits & fixtureB.GetFilterData().categoryBits && fixtureA.GetFilterData().categoryBits & maskBits))
				{
					handler(fixtureA, fixtureB);
				}
			})
		{
		}

		const std::function<void(b2Fixture&, b2Fixture&)> handler;
	};
}
