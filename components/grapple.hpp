#pragma once

#include "components/dynamicWall.hpp"

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

		void setComponentId(ComponentId id) override
		{
			ComponentBase::setComponentId(id);
			setBodyComponentVariant(TCM::Grapple(id, this));
		}
	};
}
