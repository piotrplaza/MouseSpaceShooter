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
			RenderingSetupF renderingSetupF = nullptr,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<ShadersUtils::ProgramId> customShadersProgram = std::nullopt):
			Physical(std::move(body), texture, std::move(renderingSetupF), renderLayer, customShadersProgram),
			thrustForce(thrustForce)
		{
		}

		float thrustForce;

		void init(ComponentId id, bool static_) override
		{
			ComponentBase::init(id, static_);
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::projectile, Globals::CollisionBits::all);
			setBodyComponentVariant(CM::Missile(*this));
		}

		void step() override
		{
			if (stepF)
				stepF();

			body->ApplyForceToCenter(b2Vec2(std::cos(body->GetAngle()), std::sin(body->GetAngle())) * thrustForce, true);
		}
	};
}
