#pragma once

#include <functional>

#include <glm/vec2.hpp>

#include <componentBase.hpp>

#include <commonTypes/resolutionMode.hpp>

namespace Components
{
	struct LowResBuffers : ComponentBase
	{
		struct SubBuffers
		{
			SubBuffers(LowResBuffers& lowResBuffers)
			{
				++lowResBuffers.instances;
			}

			unsigned fbo = 0;
			unsigned textureUnit = 0;
			unsigned textureObject = 0;

			glm::ivec2 size = { 0, 0 };
		};

		LowResBuffers() :
			dummy(*this),
			lowerLinear(*this),
			lowestLinear(*this),
			pixelArt(*this)
		{
		}

		const SubBuffers& getSubBuffers(ResolutionMode resolutionMode) const
		{
			switch (resolutionMode)
			{
			case ResolutionMode::Normal:
				return dummy;
			case ResolutionMode::LowerLinear:
				return lowerLinear;
			case ResolutionMode::LowestLinear:
				return lowestLinear;
			case ResolutionMode::PixelArt:
				return pixelArt;
			default:
				assert(!"unsupported resolution mode");
				return dummy;
			}
		}

		int instances = -1;

		SubBuffers dummy;
		SubBuffers lowerLinear;
		SubBuffers lowestLinear;
		SubBuffers pixelArt;
	};
}
