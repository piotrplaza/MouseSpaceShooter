#pragma once

#include "staticPolyline.hpp"

#include <tools/b2Helpers.hpp>

#include <ogl/buffers/genericBuffers.hpp>

#include <optional>

namespace Components
{
	struct DynamicPolyline : StaticPolyline
	{
		using StaticPolyline::StaticPolyline;

		void setComponentId(ComponentId id) override
		{
			ComponentBase::setComponentId(id);
			setBodyComponentVariant(TCM::DynamicPolyline(id, this));
		}
	};
}
