#pragma once

#include "staticWall.hpp"

namespace Components
{
	struct DynamicWall : StaticWall
	{
		using StaticWall::StaticWall;

		void setComponentId(ComponentId id) override
		{
			ComponentBase::setComponentId(id);
			setBodyComponentVariant(TCM::DynamicWall(id, this));
		}
	};
}
