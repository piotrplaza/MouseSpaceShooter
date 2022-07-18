#pragma once

#include "_componentBase.hpp"

#include <commonTypes/resolutionMode.hpp>

#include <glm/vec2.hpp>

#include <functional>

namespace Components
{
	struct Framebuffers : ComponentBase
	{
		struct SubBuffers
		{
			unsigned fbo = 0;
			unsigned depthBuffer = 0;
			unsigned textureUnit = 0;
			unsigned textureObject = 0;

			glm::ivec2 size = { 0, 0 };
		};

		const SubBuffers& getSubBuffers(ResolutionMode resolutionMode) const
		{
			switch (resolutionMode)
			{
			case ResolutionMode::Normal:
				return dummy;
			case ResolutionMode::LowerLinearBlend0:
				return lowerLinearBlend0;
			case ResolutionMode::LowerLinearBlend1:
				return lowerLinearBlend1;
			case ResolutionMode::LowestLinearBlend0:
				return lowestLinearBlend0;
			case ResolutionMode::LowestLinearBlend1:
				return lowestLinearBlend1;
			case ResolutionMode::PixelArtBlend0:
				return pixelArtBlend0;
			case ResolutionMode::PixelArtBlend1:
				return pixelArtBlend1;
			case ResolutionMode::LowPixelArtBlend0:
				return lowPixelArtBlend0;
			case ResolutionMode::LowPixelArtBlend1:
				return lowPixelArtBlend1;
			default:
				assert(!"unsupported resolution mode");
				return dummy;
			}
		}

		SubBuffers dummy;
		SubBuffers main;
		SubBuffers lowerLinearBlend0;
		SubBuffers lowerLinearBlend1;
		SubBuffers lowestLinearBlend0;
		SubBuffers lowestLinearBlend1;
		SubBuffers pixelArtBlend0;
		SubBuffers pixelArtBlend1;
		SubBuffers lowPixelArtBlend0;
		SubBuffers lowPixelArtBlend1;
	};
}
