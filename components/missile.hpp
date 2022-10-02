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
			TextureComponentVariant texture = std::monostate{},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			Physical(std::move(body), texture, renderingSetup, renderLayer, customShadersProgram),
			thrustForce(thrustForce)
		{
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::missile, Globals::CollisionBits::all);
		}

		float thrustForce;

		void setComponentId(ComponentId id) override
		{
			ComponentBase::setComponentId(id);
			setBodyComponentVariant(TCM::Missile(id, this));
		}

		void step() override
		{
			body->ApplyForceToCenter(b2Vec2(std::cos(body->GetAngle()), std::sin(body->GetAngle())) * thrustForce, true);
		}
	};
}
