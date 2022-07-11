#pragma once

#include "_componentBase.hpp"

#include <ogl/buffers/genericBuffers.hpp>

#include <commonTypes/buffersCollections.hpp>

#include <vector>
#include <unordered_map>

namespace Components
{
	struct RenderingBuffers : ComponentBase
	{
		BuffersColections<std::vector<Buffers::GenericBuffers>> staticBuffers;
		BuffersColections<std::unordered_map<ComponentId, Buffers::GenericBuffers>> dynamicBuffers;
	};
}
