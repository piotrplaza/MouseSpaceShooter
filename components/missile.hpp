#pragma once

#include "_physical.hpp"

#include <tools/b2Helpers.hpp>
#include <globals/collisionBits.hpp>

namespace Components
{
	struct Missile : Physical
	{
		Missile(Body body,
			float thrustForce = 5.0f,
			AbstractTextureComponentVariant texture = std::monostate{},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			Physical(std::move(body), texture, renderingSetup, renderLayer, customShadersProgram),
			thrustForce(thrustForce)
		{
		}

		float thrustForce;

		void init(ComponentId id) override
		{
			ComponentBase::init(id);
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::missile, Globals::CollisionBits::all);
			setBodyComponentVariant(TCM::Missile(id, this));
		}

		void step() override
		{
			if (stepF)
				stepF();

			body->ApplyForceToCenter(b2Vec2(std::cos(body->GetAngle()), std::sin(body->GetAngle())) * thrustForce, true);
		}
	};
}
