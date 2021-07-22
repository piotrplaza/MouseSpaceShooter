#pragma once

#include <vector>
#include <optional>
#include <functional>

#include <glm/glm.hpp>

#include <Box2D/Box2D.h>

#include <componentBase.hpp>

#include <commonTypes/resolutionMode.hpp>

#include <ogl/shaders.hpp>
#include <ogl/renderingSetup.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/b2Helpers.hpp>

#include <bodyUserData.hpp>

namespace Components
{
	struct Missile : ComponentBase
	{
		Missile(Body body, std::optional<unsigned> texture = std::nullopt,
			Tools::UniqueRenderingSetup renderingSetup = nullptr,
			std::unique_ptr<Tools::TextureAnimationController> animationController = nullptr,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			body(std::move(body)),
			texture(texture),
			renderingSetup(std::move(renderingSetup)),
			animationController(std::move(animationController)),
			customShadersProgram(customShadersProgram)
		{
			Tools::AccessUserData(*this->body).componentId = getComponentId();
		}

		Body body;
		std::optional<unsigned> texture;
		Tools::UniqueRenderingSetup renderingSetup;
		std::unique_ptr<Tools::TextureAnimationController> animationController;
		std::optional<Shaders::ProgramId> customShadersProgram;
		std::function<void()> step;
		ResolutionMode resolutionMode = ResolutionMode::Normal;

		static constexpr GLenum drawMode = GL_TRIANGLES;
		static constexpr GLenum bufferDataUsage = GL_STATIC_DRAW;

		glm::vec2 getCenter() const
		{
			return ToVec2<glm::vec2>(body->GetWorldCenter());
		}

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
