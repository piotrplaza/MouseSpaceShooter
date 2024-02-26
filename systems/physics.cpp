#include "physics.hpp"

#include <components/physics.hpp>
#include <components/collisionHandler.hpp>
#include <components/collisionFilter.hpp>
#include <components/screenInfo.hpp>

#include <globals/components.hpp>

#include <Box2D/Box2D.h>

#define FORCE_REFRESH_RATE_BASED_STEP 0

namespace
{
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
	Physics::Physics()
		: world(new b2World({0.0f, 0.0f}))
	{
		Globals::Components().physics().world = world.get();
	}

	Physics::~Physics() = default;

	void Physics::postInit()
	{
		world->SetContactListener(&contactListener);
		world->SetContactFilter(&contactFilter);
		Globals::Components().physics().prevFrameTime = std::chrono::high_resolution_clock::now();
	}

	void Physics::step()
	{
		const auto& screenInfo = Globals::Components().screenInfo();
		auto& physics = Globals::Components().physics();
		const auto currentTime = std::chrono::high_resolution_clock::now();

#if defined _DEBUG || FORCE_REFRESH_RATE_BASED_STEP
		physics.frameDuration = physics.gameSpeed * screenInfo.getRefreshDuration();
#else
		if (physics.forceRefreshRateBasedStep)
			physics.frameDuration = physics.gameSpeed * screenInfo.getRefreshDuration();
		else
		{
			physics.frameDuration = physics.gameSpeed * std::chrono::duration<float>(currentTime - physics.prevFrameTime).count();
			if (1.0f / physics.frameDuration < physics.minFPS)
				physics.frameDuration = 1.0f / physics.minFPS;
		}
#endif
		physics.prevFrameTime = currentTime;
		physics.simulationDuration += physics.frameDuration;
		world->Step(physics.frameDuration, physics.velocityIterationsPerStep, physics.positionIterationsPerStep);
	}

	void Physics::pause()
	{
		Globals::Components().physics().prevFrameTime = std::chrono::high_resolution_clock::now();
	}
}
