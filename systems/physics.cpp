#include "physics.hpp"

#include <components/physics.hpp>
#include <components/collisionHandler.hpp>
#include <components/collisionFilter.hpp>

#include <globals/components.hpp>

namespace
{
	constexpr float debugFrameDuration = 1.0f / 144;

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
				collisionHandler.second.rawHandler(*contact->GetFixtureA(), *contact->GetFixtureB());
			}
		}
	} contactListener;

	class ContactFilter : public b2ContactFilter
	{
		bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) override
		{
			for (auto& [id, collisionFilter] : Globals::Components().collisionFilters())
			{
				const auto result = collisionFilter.rawFilter(*fixtureA, *fixtureB);
				if (result != Components::CollisionFilter::Result::Fallback)
					return (bool)result;
			}
			return b2ContactFilter::ShouldCollide(fixtureA, fixtureB);
		}
	} contactFilter;
}

namespace Systems
{
	Physics::Physics() = default;

	void Physics::postInit()
	{
		Globals::Components().physics().world->SetContactListener(&contactListener);
		Globals::Components().physics().world->SetContactFilter(&contactFilter);
#ifndef _DEBUG 
		Globals::Components().physics().prevFrameTime = std::chrono::high_resolution_clock::now();
#endif
	}

	void Physics::step()
	{
		auto& physics = Globals::Components().physics();
		const auto currentTime = std::chrono::high_resolution_clock::now();
#ifndef _DEBUG
		physics.frameDuration = !physics.paused * (std::chrono::duration<float>(currentTime - physics.prevFrameTime).count()) * physics.gameSpeed;
		physics.prevFrameTime = currentTime;
#else
		physics.frameDuration = !physics.paused * debugFrameDuration * physics.gameSpeed;
#endif

		if (physics.paused)
			return;

		physics.simulationDuration += physics.frameDuration;
		physics.world->Step(physics.frameDuration, 3, 8);
	}
}
