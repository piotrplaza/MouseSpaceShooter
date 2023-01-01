#pragma once

#include "components/wall.hpp"

namespace Components
{
	struct Grapple : DynamicWall
	{
		using DynamicWall::DynamicWall;

		float influenceRadius = 0.0f;

		struct
		{
			glm::vec2 previousCenter{ 0.0f, 0.0f };
		} details;

		void init(ComponentId id) override
		{
			ComponentBase::init(id);
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::wall, Globals::CollisionBits::all);
			setBodyComponentVariant(TCM::Grapple(id, this));
		}
	};
}
