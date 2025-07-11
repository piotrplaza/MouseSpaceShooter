#include "physics.hpp"

#include <components/physics.hpp>
#include <components/collisionHandler.hpp>
#include <components/collisionFilter.hpp>
#include <components/systemInfo.hpp>

#include <globals/components.hpp>

#define FORCE_REFRESH_RATE_OR_TIME_BASED_STEP 0

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
				collisionHandler.rawHandler(*contact->GetFixtureA(), *contact->GetFixtureB());
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
	{
		Globals::Components().physics().world = std::make_unique<b2World>(b2Vec2(0.0f, 0.0f));
	}

	Physics::~Physics() = default;

	void Physics::postInit()
	{
		Globals::Components().physics().world->SetContactListener(&contactListener);
		Globals::Components().physics().world->SetContactFilter(&contactFilter);
		Globals::Components().physics().prevFrameTime = std::chrono::high_resolution_clock::now();
	}

	void Physics::teardown()
	{
		extern b2ContactListener b2_defaultListener;
		extern b2ContactFilter b2_defaultFilter;

		Globals::Components().physics().world->SetContactListener(&b2_defaultListener);
		Globals::Components().physics().world->SetContactFilter(&b2_defaultFilter);
	}

	void Physics::step()
	{
		auto& physics = Globals::Components().physics();

		if (Globals::Components().physics().paused)
		{
			physics.frameDuration = 0.0f;
			physics.prevFrameTime = std::chrono::high_resolution_clock::now();
			return;
		}

		const auto& screenInfo = Globals::Components().systemInfo().screen;
		const auto currentTime = std::chrono::high_resolution_clock::now();

		auto refreshRateBasedStep = [&]() {
			physics.frameDuration = physics.gameSpeed * screenInfo.getRefreshDuration();
		};

		auto timeBasedStep = [&]() {
			physics.frameDuration = physics.gameSpeed * std::chrono::duration<float>(currentTime - physics.prevFrameTime).count();
			if (1.0f / physics.frameDuration < physics.minFPS)
				physics.frameDuration = 1.0f / physics.minFPS;
		};

#if FORCE_REFRESH_RATE_OR_TIME_BASED_STEP == 1
		refreshRateBasedStep();
#elif FORCE_REFRESH_RATE_OR_TIME_BASED_STEP == 2
		timeBasedStep();
#else
		if (physics.forceRefreshRateOrTimeBasedStep == 1)
			refreshRateBasedStep();
		else if (physics.forceRefreshRateOrTimeBasedStep == 2)
			timeBasedStep();
		else
#if defined _DEBUG
			refreshRateBasedStep();
#else
			timeBasedStep();
#endif
#endif

		physics.prevFrameTime = currentTime;
		physics.simulationDuration += physics.frameDuration;
		++physics.frameCount;
		physics.world->Step(physics.frameDuration, physics.velocityIterationsPerStep, physics.positionIterationsPerStep);

		physics.step();
	}
}
