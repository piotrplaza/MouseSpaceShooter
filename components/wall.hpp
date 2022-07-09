#pragma once

#include "_componentBase.hpp"
#include "_typeComponentMappers.hpp"

#include "decoration.hpp"

#include <tools/b2Helpers.hpp>
#include <tools/graphicsHelpers.hpp>

#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/bodyUserData.hpp>

#include <ogl/shaders.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <Box2D/Box2D.h>

#include <memory>
#include <vector>
#include <optional>
#include <functional>
#include <variant>

namespace Components
{
	struct Wall : ComponentBase
	{
		using ComponentBase::ComponentBase;

		Wall(Body body,
			TextureComponentVariant texture = std::monostate{},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			body(std::move(body)),
			texture(texture),
			renderingSetup(renderingSetup),
			renderLayer(renderLayer),
			customShadersProgram(customShadersProgram)
		{
			Tools::AccessUserData(*this->body).bodyComponentVariant = TCM::Wall(getComponentId());
		}

		Body body;

		std::vector<glm::vec2> texCoord;
		std::vector<glm::vec4> colors;

		TextureComponentVariant texture;
		std::optional<ComponentId> renderingSetup;
		std::optional<Shaders::ProgramId> customShadersProgram;
		std::function<void()> step;
		ResolutionMode resolutionMode = ResolutionMode::Normal;
		RenderLayer renderLayer = RenderLayer::Midground;

		GLenum drawMode = GL_TRIANGLES;
		GLenum bufferDataUsage = GL_STATIC_DRAW;

		bool preserveTextureRatio = false;

		bool render = true;

		std::vector<DecorationDef> subsequence;
		unsigned posInSubsequence = 0;

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
			const auto vertices = getVertices();
			if (texCoord.empty())
			{
				return std::vector<glm::vec2>(vertices.begin(), vertices.end());
			}
			else if (texCoord.size() < vertices.size())
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
