#pragma once

#include "componentBase.hpp"

namespace Components
{
	struct BlendingTexture : ComponentBase
	{
		using ComponentBase::ComponentBase;

		BlendingTexture(std::initializer_list<ComponentId> texturesIds):
			texturesIds(texturesIds)
		{
		}

		BlendingTexture(std::vector<ComponentId> texturesIds) :
			texturesIds(std::move(texturesIds))
		{
		}

		std::vector<ComponentId> texturesIds;
	};
}
