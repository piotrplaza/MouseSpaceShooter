#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <functional>

#include <Box2D/Box2D.h>

#include <componentBase.hpp>

#include <commonTypes/resolutionMode.hpp>

#include <ogl/shaders.hpp>
#include <ogl/renderingSetup.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/graphicsHelpers.hpp>

#include <bodyUserData.hpp>

namespace Components
{
	struct Wall : ComponentBase
	{
		Wall(Body body, std::optional<unsigned> texture = std::nullopt,
			Tools::UniqueRenderingSetup renderingSetup = nullptr,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			body(std::move(body)),
			texture(texture),
			renderingSetup(std::move(renderingSetup)),
			customShadersProgram(customShadersProgram)
		{
			Tools::AccessUserData(*this->body).componentId = componentId;
		}

		Body body;
		std::optional<unsigned> texture;
		Tools::UniqueRenderingSetup renderingSetup;
		std::optional<Shaders::ProgramId> customShadersProgram;
		ResolutionMode resolutionMode = ResolutionMode::Normal;

		GLenum drawMode = GL_TRIANGLES;
		GLenum bufferDataUsage = GL_STATIC_DRAW;

		std::vector<glm::vec3> getPositions() const
		{
			return Tools::GetPositions(*body);
		}

		std::vector<glm::vec3> getTransformedPositions() const
		{
			return Tools::Transform(getPositions(), getModelMatrix());
		}

		const std::vector<glm::vec2> getTexCoord() const
		{
			const auto positions = getPositions();
			return std::vector<glm::vec2>(positions.begin(), positions.end());
		}

		glm::mat4 getModelMatrix() const
		{
			return Tools::GetModelMatrix(*body);
		}

		bool isTextureRatioPreserved() const
		{
			return true;
		}
	};
}
