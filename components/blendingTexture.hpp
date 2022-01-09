#pragma once

#include "componentBase.hpp"

namespace Components
{
	struct BlendingTexture : ComponentBase
	{
		using ComponentBase::ComponentBase;

		BlendingTexture(ComponentId controlTexture, ComponentId textureR, ComponentId textureG = 0, ComponentId textureB = 0, ComponentId textureA = 0):
			controlTexture(controlTexture),
			textureR(textureR),
			textureG(textureG),
			textureB(textureB),
			textureA(textureA)
		{
		}

		ComponentId controlTexture = 0;
		ComponentId textureR = 0;
		ComponentId textureG = 0;
		ComponentId textureB = 0;
		ComponentId textureA = 0;
	};
}
