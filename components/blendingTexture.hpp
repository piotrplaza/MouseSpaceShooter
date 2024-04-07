#pragma once

#include "_componentBase.hpp"

#include <commonTypes/componentMappers.hpp>

#include <vector>
#include <initializer_list>

namespace Components
{
	struct BlendingTexture : ComponentBase
	{
		BlendingTexture(std::initializer_list<AbstractTextureComponentVariant> textures):
			textures(textures)
		{
		}

		BlendingTexture(std::vector<AbstractTextureComponentVariant> textures):
			textures(std::move(textures))
		{
		}

		std::vector<AbstractTextureComponentVariant> textures;
	};
}
