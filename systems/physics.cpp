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
		prevFrameTime = std::chrono::high_resolution_clock::now();
#endif
	}

	void Physics::step()
	{
		auto& physics = Globals::Components().physics();
		const auto currentTime = std::chrono::high_resolution_clock::now();
#ifndef _DEBUG
		physics.frameDuration = !physics.paused * (std::chrono::duration<float>(currentTime - prevFrameTime).count()) * physics.gameSpeed;
		prevFrameTime = currentTime;
#else
		physics.frameDuration = !physics.paused * debugFrameDuration * physics.gameSpeed;
#endif
		physics.simulationDuration += physics.frameDuration;
		physics.world->Step(physics.frameDuration, 3, 8);
	}
}
