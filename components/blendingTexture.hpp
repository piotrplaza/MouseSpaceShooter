#pragma once

#include "componentBase.hpp"

namespace Components
{
	struct BlendingTexture : ComponentBase
	{
		using ComponentBase::ComponentBase;

		BlendingTexture(std::initializer_list<ComponentId> texturesIds, bool blendingAnimation = false):
			texturesIds(texturesIds),
			blendingAnimation(blendingAnimation)
		{
		}

		BlendingTexture(std::vector<ComponentId> texturesIds, bool blendingAnimation = false):
			texturesIds(std::move(texturesIds)),
			blendingAnimation(blendingAnimation)
		{
		}

		std::vector<ComponentId> texturesIds;
		bool blendingAnimation;
	};
}
