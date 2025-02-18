#pragma once

#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/renderLayer.hpp>

#include <glm/vec2.hpp>

#include <optional>

namespace Components
{
	struct Defaults
	{
		ResolutionMode resolutionMode = { ResolutionMode::Resolution::Native, ResolutionMode::Scaling::Linear, ResolutionMode::Blending::Standard };
		RenderLayer renderLayer = RenderLayer::Midground;
	};
}
