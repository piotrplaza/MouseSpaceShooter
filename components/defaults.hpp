#pragma once

#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/renderLayer.hpp>

namespace Components
{
	struct Defaults
	{
		ResolutionMode resolutionMode = {};
		RenderLayer renderLayer = RenderLayer::Midground;
	};
}
