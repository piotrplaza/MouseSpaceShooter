#pragma once

#include "componentBase.hpp"
#include "typeComponentMappers.hpp"

#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/renderLayer.hpp>

#include <ogl/shaders.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <memory>
#include <vector>
#include <functional>
#include <optional>
#include <variant>

namespace Components
{
	struct DecorationDef
	{
		DecorationDef(std::vector<glm::vec3> vertices = {},
			TextureComponentVariant texture = std::monostate{},
			std::vector<glm::vec2> texCoord = {},
			std::optional<ComponentId> renderingSetup = std::nullopt) :
			vertices(std::move(vertices)),
			texture(texture),
			texCoord(std::move(texCoord)),
			renderingSetup(renderingSetup)
		{
		}

		std::vector<glm::vec3> vertices;
		std::vector<glm::vec4> colors;
		std::vector<glm::vec2> texCoord;

		TextureComponentVariant texture;
		std::optional<ComponentId> renderingSetup;
		std::function<glm::mat4()> modelMatrixF;

		GLenum drawMode = GL_TRIANGLES;
		GLenum bufferDataUsage = GL_STATIC_DRAW;

		bool preserveTextureRatio = false;

		bool render = true;

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

	struct Decoration : ComponentBase, DecorationDef
	{
		using ComponentBase::ComponentBase;

		Decoration(std::vector<glm::vec3> vertices = {},
			TextureComponentVariant texture = std::monostate{},
			std::vector<glm::vec2> texCoord = {},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::NearMidground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt) :
			DecorationDef(std::move(vertices), texture, std::move(texCoord), renderingSetup),
			renderLayer(renderLayer),
			customShadersProgram(customShadersProgram)
		{
		}

		std::optional<Shaders::ProgramId> customShadersProgram;
		std::function<void()> step;
		ResolutionMode resolutionMode = ResolutionMode::Normal;
		RenderLayer renderLayer = RenderLayer::NearMidground;

		std::vector<DecorationDef> subsequence;
		unsigned posInSubsequence = 0;
	};
}
