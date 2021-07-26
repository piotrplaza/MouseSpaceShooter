#include "physics.hpp"

#include <globals.hpp>

#include <components/physics.hpp>
#include <components/collisionHandler.hpp>

namespace
{
	constexpr float debugFrameDuration = 1.0f / 160;

	class ContactListener : public b2ContactListener
	{
		void BeginContact(b2Contact* contact) override
		{
			Contact(contact, Globals::Components().beginCollisionHandlers());
		}

		void EndContact(b2Contact* contact) override
		{
			Contact(contact, Globals::Components().endCollisionHandlers());
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
		Globals::Components().physics().world->SetContactListener(&contactListener);
	}

	void Physics::step()
	{
		auto& physics = Globals::Components().physics();

		if (firstStep)
		{
#ifndef _DEBUG 
			startTime = std::chrono::high_resolution_clock::now();
#endif
			firstStep = false;
		}

#ifndef _DEBUG 
		const auto simulationDuration = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - startTime).count() - pauseDuration;
		physics.frameDuration = simulationDuration - physics.simulationDuration;
		physics.simulationDuration = simulationDuration;
#else
		physics.frameDuration = debugFrameDuration;
		physics.simulationDuration += physics.frameDuration;
#endif

		physics.world->Step(physics.frameDuration, 3, 8);
	}

	void Physics::pause()
	{
		if (!pauseTime)
			pauseTime = std::chrono::high_resolution_clock::now();
	}

	void Physics::resume()
	{
		if (pauseTime)
		{
			pauseDuration += std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - *pauseTime).count();
			pauseTime.reset();
		}
	}
}
