#pragma once

#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/renderLayer.hpp>

#include <optional>

namespace Components
{
	struct Defaults
	{
		ResolutionMode resolutionMode = {};
		RenderLayer renderLayer = RenderLayer::Midground;

		std::optional<ResolutionMode> forcedResolutionMode = {};
	};
}
