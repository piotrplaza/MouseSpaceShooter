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
			Contact(contact, Globals::Components::beginCollisionHandlers);
		}

		void EndContact(b2Contact* contact) override
		{
			Contact(contact, Globals::Components::endCollisionHandlers);
		}

		void Contact(b2Contact* contact, std::unordered_map<::ComponentId, ::Components::CollisionHandler>& collisionHandlers)
		{
			for (auto& collisionHandler : collisionHandlers)
			{
				collisionHandler.second.handler(*contact->GetFixtureA(), *contact->GetFixtureB());
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
			startPoint = std::chrono::high_resolution_clock::now();
#endif
			firstStep = false;
		}

#ifndef _DEBUG 
		const auto simulationTime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - startPoint).count() - pauseTime;
		physics.frameTime = simulationTime - physics.simulationTime;
		physics.simulationTime = simulationTime;
#else
		physics.frameTime = debugFrameTime;
		physics.simulationTime += physics.frameTime;
#endif

		physics.world.Step(physics.frameTime, 3, 8);
	}

	void Physics::pause()
	{
		if (!pausePoint)
			pausePoint = std::chrono::high_resolution_clock::now();
	}

	void Physics::resume()
	{
		if (pausePoint)
		{
			pauseTime += std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - *pausePoint).count();
			pausePoint.reset();
		}
	}
}
