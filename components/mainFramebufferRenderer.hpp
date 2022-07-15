#pragma once

#include "_componentBase.hpp"

#include <functional>

namespace Components
{
	struct MainFramebufferRenderer : ComponentBase
	{
		std::function<void(unsigned textureId)> renderer;

		inline void operator()(unsigned textureId)
		{
			renderer(textureId);
		}
	};
}
