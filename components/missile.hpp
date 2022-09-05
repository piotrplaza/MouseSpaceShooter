#pragma once

#include "_componentBase.hpp"
#include "_physical.hpp"

#include <tools/b2Helpers.hpp>
#include <globals/collisionBits.hpp>

namespace Components
{
	struct Missile : ComponentBase, Physical
	{
		Missile(Body body,
			TextureComponentVariant texture = std::monostate{},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			Physical(std::move(body), TCM::Missile(getComponentId()), texture, renderingSetup, renderLayer, customShadersProgram)
		{
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::missile, Globals::CollisionBits::all);
			this->body->SetBullet(true);
		}

		std::function<void()> step;
	};
}
