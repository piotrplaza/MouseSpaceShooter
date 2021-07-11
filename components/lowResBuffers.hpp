#pragma once

#include <functional>

#include <glm/vec2.hpp>

#include <componentBase.hpp>

namespace Components
{
	struct LowResBuffers : ComponentBase
	{
		unsigned fbo;
		unsigned textureUnit;
		unsigned textureObject;

		glm::ivec2 size = { 0, 0 };;
	};
}
