#pragma once

#include <commonTypes/standardRenderMode.hpp>
#include <commonTypes/renderLayer.hpp>

#include <glm/vec2.hpp>

#include <optional>

namespace Components
{
	struct Defaults
	{
		StandardRenderMode resolutionMode;
		RenderLayer renderLayer = RenderLayer::Midground;
		float soundVolume = 1.0f;
		float soundMinDistance = 4.0f;
		float soundAttenuation = 0.2f;
	};
}
