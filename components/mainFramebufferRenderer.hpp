#pragma once

#include <functional>

#include <componentBase.hpp>

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
