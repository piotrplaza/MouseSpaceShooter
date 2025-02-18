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
			unsigned textureId = 0;
			unsigned textureObject = 0;

			glm::ivec2 size = { 0, 0 };
		};

		const SubBuffers& getSubBuffers(ResolutionMode resolutionMode) const
		{
			//if (resolutionMode.isDefaultMode())
			//	return dummy;

			return subBuffers[(size_t)resolutionMode.resolution][(size_t)resolutionMode.scaling][(size_t)resolutionMode.blending];
		}

		const SubBuffers& getDefaultSubBuffers() const
		{
			return subBuffers[(size_t)ResolutionMode::defaultResolution][(size_t)ResolutionMode::defaultScaling][(size_t)ResolutionMode::defaultBlending];
		}

		//SubBuffers dummy;
		SubBuffers subBuffers[(size_t)ResolutionMode::Resolution::COUNT][(size_t)ResolutionMode::Scaling::COUNT][(size_t)ResolutionMode::Blending::COUNT];
	};
}
