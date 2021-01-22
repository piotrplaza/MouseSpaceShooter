#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <functional>

#include <Box2D/Box2D.h>

#include <componentBase.hpp>

#include <ogl/shaders.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/graphicsHelpers.hpp>

#include <bodyUserData.hpp>

namespace Components
{
	struct Wall : ComponentBase
	{
		using RenderingSetup = std::function<std::function<void()>(Shaders::ProgramId)>;

		Wall(Body body, std::optional<unsigned> texture = std::nullopt,
			std::unique_ptr<RenderingSetup> renderingSetup = nullptr,
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
		std::unique_ptr<RenderingSetup> renderingSetup;
		std::optional<Shaders::ProgramId> customShadersProgram;

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
