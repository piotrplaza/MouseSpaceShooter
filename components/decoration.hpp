#pragma once

#include "_componentBase.hpp"
#include "_renderable.hpp"

#include <commonTypes/resolutionMode.hpp>

namespace Components
{
	struct Decoration : ComponentBase, Renderable
	{
		Decoration(std::vector<glm::vec3> vertices = {},
			TextureComponentVariant texture = std::monostate{},
			std::vector<glm::vec2> texCoord = {},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::NearMidground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt) :
			Renderable(texture, renderingSetup, renderLayer, customShadersProgram, std::move(vertices), {}, std::move(texCoord))
		{
		}

		std::function<void()> step;
		std::function<glm::mat4()> modelMatrixF;

		const std::vector<glm::vec3>& getVertices() const
		{
			return vertices;
		}

		const std::vector<glm::vec4>& getColors() const
		{
			return colors;
		}

		const std::vector<glm::vec2> getTexCoord() const
		{
			if (texCoord.empty())
			{
				return std::vector<glm::vec2>(getVertices().begin(), getVertices().end());
			}
			else if (texCoord.size() < getVertices().size())
			{
				std::vector<glm::vec2> cyclicTexCoord;
				cyclicTexCoord.reserve(getVertices().size());
				for (size_t i = 0; i < getVertices().size(); ++i)
					cyclicTexCoord.push_back(texCoord[i % texCoord.size()]);
				return cyclicTexCoord;
			}
			else
			{
				assert(texCoord.size() == getVertices().size());
				return texCoord;
			}
		}

		glm::mat4 getModelMatrix() const
		{
			return modelMatrixF ? modelMatrixF() : glm::mat4(1.0f);
		}
	};
}
