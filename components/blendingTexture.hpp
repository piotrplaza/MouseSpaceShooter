#pragma once

#include <componentBase.hpp>

namespace Components
{
	struct BlendingTexture : ComponentBase
	{
		using ComponentBase::ComponentBase;

		int blendingTexture = -1;
		int textureR = -1;
		int textureG = -1;
		int textureB = -1;
		int textureA = -1;
	};
}
