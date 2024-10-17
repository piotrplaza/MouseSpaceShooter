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
			if (resolutionMode.isMainMode())
				return dummy;

			return subBuffers[(size_t)resolutionMode.resolution][(size_t)resolutionMode.scaling][(size_t)resolutionMode.blending];
		}

		const SubBuffers& getMainSubBuffers() const
		{
			return subBuffers[(size_t)ResolutionMode::Resolution::Native][(size_t)ResolutionMode::Scaling::Linear][(size_t)ResolutionMode::Blending::Standard];
		}

		SubBuffers dummy;
		SubBuffers subBuffers[(size_t)ResolutionMode::Resolution::COUNT][(size_t)ResolutionMode::Scaling::COUNT][(size_t)ResolutionMode::Blending::COUNT];
	};
}
