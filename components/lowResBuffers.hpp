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
			unsigned fbo;
			unsigned textureUnit;
			unsigned textureObject;

			glm::ivec2 size = { 0, 0 };
		};

		const SubBuffers& getSubBuffers(ResolutionMode resolutionMode) const
		{
			switch (resolutionMode)
			{
			case ResolutionMode::Normal:
				return dummy;
			case ResolutionMode::LowLinear:
				return lowLinear;
			case ResolutionMode::PixelArt:
				return pixelArt;
			default:
				assert(!"unsupported resolution mode");
				return dummy;
			}
		}

		SubBuffers dummy;
		SubBuffers lowLinear;
		SubBuffers pixelArt;
	};
}
