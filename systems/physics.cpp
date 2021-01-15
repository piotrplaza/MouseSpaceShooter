#include "physics.hpp"

#include <globals.hpp>

#include <components/physics.hpp>
#include <components/collisionHandler.hpp>

namespace
{
	constexpr float debugFrameTime = 1.0f / 160;

	class ContactListener : public b2ContactListener
	{
		void BeginContact(b2Contact* contact) override
		{
			using namespace Globals::Components;

			for (auto& beginCollisionHandler : beginCollisionHandlers)
			{
				beginCollisionHandler.second.handler(*contact->GetFixtureA(), *contact->GetFixtureB());
			}
		}

		void EndContact(b2Contact* contact) override
		{
			using namespace Globals::Components;

			for (auto& endCollisionHandler : endCollisionHandlers)
			{
				endCollisionHandler.second.handler(*contact->GetFixtureA(), *contact->GetFixtureB());
			}
		}
	} contactListener;
}

namespace Systems
{
	Physics::Physics()
	{
		using namespace Globals::Components;

		physics.world.SetContactListener(&contactListener);
	}

	void Physics::step()
	{
		using namespace Globals::Components;

		if (firstStep)
		{
#ifndef _DEBUG 
			start = std::chrono::high_resolution_clock::now();
#endif
			firstStep = false;
		}

#ifndef _DEBUG 
		const auto simulationTime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();
		physics.frameTime = simulationTime - physics.simulationTime;
		physics.simulationTime = simulationTime;
#else
		physics.frameTime = debugFrameTime;
		physics.simulationTime += physics.frameTime;
#endif

		physics.world.Step(physics.frameTime, 3, 8);
	}
}
