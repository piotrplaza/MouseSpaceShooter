#pragma once

#include "_componentBase.hpp"
#include "_renderable.hpp"

#include <tools/graphicsHelpers.hpp>
#include <tools/b2Helpers.hpp>

#include <commonTypes/bodyUserData.hpp>

namespace Components
{
	struct Missile : ComponentBase, Renderable
	{
		Missile() = default;

		Missile(Body body,
			TextureComponentVariant texture = std::monostate{},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			Renderable(texture, renderingSetup, renderLayer, customShadersProgram),
			body(std::move(body))
		{
			Tools::AccessUserData(*this->body).bodyComponentVariant = TCM::Missile(getComponentId());
		}

		Body body;
		
		std::function<void()> step;

		std::vector<glm::vec3> getVertices() const
		{
			return Tools::GetVertices(*body);
		}

		const std::vector<glm::vec4>& getColors() const
		{
			return colors;
		}

		const std::vector<glm::vec2> getTexCoord() const
		{
			if (texCoord.empty())
			{
				const auto vertices = getVertices();
				return std::vector<glm::vec2>(vertices.begin(), vertices.end());
			}
			else
			{
				const auto vertices = getVertices();
				if (texCoord.size() < vertices.size())
				{
					std::vector<glm::vec2> cyclicTexCoord;
					cyclicTexCoord.reserve(vertices.size());
					for (size_t i = 0; i < vertices.size(); ++i)
						cyclicTexCoord.push_back(texCoord[i % texCoord.size()]);
					return cyclicTexCoord;
				}
				else
				{
					assert(texCoord.size() == vertices.size());
					return texCoord;
				}
			}
		}

		glm::vec2 getCenter() const
		{
			return ToVec2<glm::vec2>(body->GetWorldCenter());
		}

		std::vector<glm::vec3> getTransformedVertices() const
		{
			return Tools::Transform(getVertices(), getModelMatrix());
		}

		glm::mat4 getModelMatrix() const
		{
			return Tools::GetModelMatrix(*body);
		}
	};
}
