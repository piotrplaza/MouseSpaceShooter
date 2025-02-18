#pragma once

#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/renderLayer.hpp>

#include <glm/vec2.hpp>

#include <optional>

namespace Components
{
	struct Defaults
	{
		ResolutionMode resolutionMode = { ResolutionMode::defaultResolution, ResolutionMode::defaultScaling, ResolutionMode::defaultBlending };
		RenderLayer renderLayer = RenderLayer::Midground;
	};
}
