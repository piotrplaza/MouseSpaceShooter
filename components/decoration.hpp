#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <optional>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <componentBase.hpp>

#include <ogl/shaders.hpp>
#include <ogl/renderingSetup.hpp>

#include <tools/animations.hpp>

namespace Components
{
	struct Decoration : ComponentBase
	{
		Decoration(std::vector<glm::vec3> positions = {}, std::optional<unsigned> texture = std::nullopt,
			Tools::UniqueRenderingSetup renderingSetup = nullptr,
			std::unique_ptr<Tools::TextureAnimationController> animationController = nullptr,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			positions(std::move(positions)),
			texture(texture),
			renderingSetup(std::move(renderingSetup)),
			animationController(std::move(animationController)),
			customShadersProgram(customShadersProgram)
		{
		}

		std::vector<glm::vec3> positions;
		std::optional<unsigned> texture;
		Tools::UniqueRenderingSetup renderingSetup;
		std::unique_ptr<Tools::TextureAnimationController> animationController;
		std::optional<Shaders::ProgramId> customShadersProgram;
		std::vector<glm::vec2> texCoord;
		std::function<void()> step;
		bool lowRes = false;
		//std::vector<float> genericBuffer;

		GLenum drawMode = GL_TRIANGLES;
		GLenum bufferDataUsage = GL_STATIC_DRAW;

		const std::vector<glm::vec3>& getPositions() const
		{
			return positions;
		}

		const std::vector<glm::vec2> getTexCoord() const
		{
			if (texCoord.empty())
			{
				const auto positions = getPositions();
				return std::vector<glm::vec2>(positions.begin(), positions.end());
			}
			else if (texCoord.size() < positions.size())
			{
				std::vector<glm::vec2> cyclicTexCoord;
				cyclicTexCoord.reserve(positions.size());
				for (size_t i = 0; i < positions.size(); ++i)
					cyclicTexCoord.push_back(texCoord[i % texCoord.size()]);
				return cyclicTexCoord;
			}
			else
			{
				assert(texCoord.size() == positions.size());
				return texCoord;
			}
		}

		bool isTextureRatioPreserved() const
		{
			return texCoord.empty();
		}
	};
}
