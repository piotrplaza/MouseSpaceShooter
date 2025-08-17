#pragma once

#include "_componentBase.hpp"

#include <commonTypes/standardRenderMode.hpp>
#include <commonTypes/componentId.hpp>

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

		const SubBuffers& getMainSubBuffers() const
		{
			return subBuffers[(size_t)StandardRenderMode::mainResolution][(size_t)StandardRenderMode::mainScaling][(size_t)StandardRenderMode::mainBlending];
		}

		const SubBuffers& getSubBuffers(const StandardRenderMode& renderMode) const
		{
			return subBuffers[(size_t)renderMode.resolution][(size_t)renderMode.scaling][(size_t)renderMode.blending];
		}

		SubBuffers subBuffers[(size_t)StandardRenderMode::Resolution::COUNT][(size_t)StandardRenderMode::Scaling::COUNT][(size_t)StandardRenderMode::Blending::COUNT];
	};
}
