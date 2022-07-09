#pragma once

#include "_componentBase.hpp"

#include <functional>

namespace Components
{
	struct MainFramebufferRenderer : ComponentBase
	{
		using ComponentBase::ComponentBase;

		std::function<void(unsigned textureId)> renderer;

		inline void operator()(unsigned textureId)
		{
			renderer(textureId);
		}
	};
}
