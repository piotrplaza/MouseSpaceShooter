#pragma once

#include "_componentBase.hpp"

#include <ogl/buffers/genericBuffers.hpp>

#include <commonTypes/buffersCollections.hpp>

#include <deque>
#include <unordered_map>

namespace Components
{
	struct RenderingBuffers : ComponentBase
	{
		BuffersColections<std::deque<Buffers::GenericBuffers>> staticBuffers;
		BuffersColections<std::unordered_map<ComponentId, Buffers::GenericBuffers>> dynamicBuffers;
		std::deque<Buffers::GenericBuffers> staticTFBuffers;
		std::unordered_map<ComponentId, Buffers::GenericBuffers> dynamicTFBuffers;
	};
}
