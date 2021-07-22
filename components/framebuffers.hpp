#pragma once

#include <functional>

#include <glm/vec2.hpp>

#include <componentBase.hpp>

#include <commonTypes/resolutionMode.hpp>

namespace Components
{
	struct Framebuffers : ComponentBase
	{
		struct SubBuffers
		{
			SubBuffers(Framebuffers& lowResBuffers)
			{
				++lowResBuffers.instances;
			}

			unsigned fbo = 0;
			unsigned textureUnit = 0;
			unsigned textureObject = 0;

			glm::ivec2 size = { 0, 0 };
		};

		Framebuffers() :
			dummy(*this),
			main(*this),
			lowerLinearBlend0(*this),
			lowerLinearBlend1(*this),
			lowestLinearBlend0(*this),
			lowestLinearBlend1(*this),
			pixelArtBlend0(*this),
			pixelArtBlend1(*this),
			lowPixelArtBlend0(*this),
			lowPixelArtBlend1(*this)
		{
		}

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

		int instances = -1;

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
