#pragma once

#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/renderLayer.hpp>

namespace Components
{
	struct Defaults
	{
		ResolutionMode resolutionMode = ResolutionMode::Normal;
		RenderLayer renderLayer = RenderLayer::Midground;
	};
}
