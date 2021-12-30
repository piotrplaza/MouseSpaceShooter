#pragma once

#include <componentBase.hpp>

namespace Components
{
	struct BlendingTexture : ComponentBase
	{
		using ComponentBase::ComponentBase;

		ComponentId blendingTexture = 0;
		ComponentId textureR = 0;
		ComponentId textureG = 0;
		ComponentId textureB = 0;
		ComponentId textureA = 0;
	};
}
