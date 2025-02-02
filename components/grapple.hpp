#pragma once

#include "components/wall.hpp"

namespace Components
{
	struct Grapple : Wall
	{
		using Wall::Wall;

		float range = 0.0f;

		struct
		{
			glm::vec2 previousCenter{ 0.0f, 0.0f };
		} details;

		void init(ComponentId id, bool static_) override
		{
			ComponentBase::init(id, static_);
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::wall, Globals::CollisionBits::all);
			setBodyComponentVariant(CM::Grapple(*this));
		}
	};
}
