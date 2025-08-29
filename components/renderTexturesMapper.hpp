#pragma once

#include "_componentBase.hpp"

#include <commonTypes/standardRenderMode.hpp>
#include <commonTypes/componentId.hpp>

#include <glm/vec2.hpp>

#include <functional>

namespace Components
{
	struct RenderTexturesMapper : ComponentBase
	{
		ComponentId renderTextureIds[(size_t)StandardRenderMode::Resolution::COUNT][(size_t)StandardRenderMode::Scaling::COUNT][(size_t)StandardRenderMode::Blending::COUNT];

		ComponentId getMainRenderTexture() const
		{
			return renderTextureIds[(size_t)StandardRenderMode::mainResolution][(size_t)StandardRenderMode::mainScaling][(size_t)StandardRenderMode::mainBlending];
		}

		ComponentId getRenderTexture(const StandardRenderMode& renderMode) const
		{
			return renderTextureIds[(size_t)renderMode.resolution][(size_t)renderMode.scaling][(size_t)renderMode.blending];
		}
	};
}
