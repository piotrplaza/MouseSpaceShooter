#include "physics.hpp"

#include <components/physics.hpp>
#include <components/collisionHandler.hpp>
#include <components/collisionFilter.hpp>

#include <globals/components.hpp>

namespace
{
	constexpr float debugFrameDuration = 1.0f / 144;

	class : public b2ContactListener
	{
		void BeginContact(b2Contact* contact) override
		{
			Contact(contact, Globals::Components().beginCollisionHandlers());
		}

		void EndContact(b2Contact* contact) override
		{
			Contact(contact, Globals::Components().endCollisionHandlers());
		}

		void Contact(b2Contact* contact, DynamicComponents<Components::CollisionHandler>& collisionHandlers)
		{
			for (auto& collisionHandler : collisionHandlers)
			{
				collisionHandler.rawHandler(*contact->GetFixtureA(), *contact->GetFixtureB());
			}
		}
	} contactListener;

	class : public b2ContactFilter
	{
		bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) override
		{
			for (const auto& collisionFilter: Globals::Components().collisionFilters())
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
#ifndef _DEBUG
		const auto currentTime = std::chrono::high_resolution_clock::now();
		physics.frameDuration = (std::chrono::duration<float>(currentTime - physics.prevFrameTime).count()) * physics.gameSpeed;
		physics.prevFrameTime = currentTime;
#else
		physics.frameDuration = debugFrameDuration * physics.gameSpeed;
#endif

		physics.simulationDuration += physics.frameDuration;
		physics.world->Step(physics.frameDuration, physics.velocityIterationsPerStep, physics.positionIterationsPerStep);
	}

	void Physics::pause()
	{
		Globals::Components().physics().prevFrameTime = std::chrono::high_resolution_clock::now();
	}
}
