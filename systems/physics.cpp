#include "physics.hpp"

#include <components/physics.hpp>
#include <components/collisionHandler.hpp>

#include <globals/components.hpp>

namespace
{
	constexpr float debugFrameDuration = 1.0f / 144;
	constexpr float gameSpeed = 1.0f;

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
	Physics::Physics() = default;

	void Physics::postInit()
	{
		Globals::Components().physics().world->SetContactListener(&contactListener);
#ifndef _DEBUG 
		startTime = std::chrono::high_resolution_clock::now();
#endif
	}

	void Physics::step()
	{
		auto& physics = Globals::Components().physics();

#ifndef _DEBUG 
		const auto simulationDuration = (std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - startTime).count() - pauseDuration) * gameSpeed;
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
