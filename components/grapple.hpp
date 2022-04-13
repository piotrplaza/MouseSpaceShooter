#pragma once

#include "componentBase.hpp"
#include "typeComponentMappers.hpp"

#include <tools/graphicsHelpers.hpp>
#include <tools/b2Helpers.hpp>

#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/bodyUserData.hpp>

#include <ogl/shaders.hpp>

#include <Box2D/Box2D.h>

#include <glm/glm.hpp>

#include <vector>
#include <optional>
#include <functional>
#include <variant>

namespace Components
{
	struct Grapple : ComponentBase
	{
		using ComponentBase::ComponentBase;

		Grapple(Body body, float influenceRadius,
			TextureComponentVariant texture = std::monostate{},
			ComponentId renderingSetup = 0,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			body(std::move(body)),
			influenceRadius(influenceRadius),
			texture(texture),
			renderingSetup(std::move(renderingSetup)),
			customShadersProgram(customShadersProgram)
		{
			Tools::AccessUserData(*this->body).bodyComponentVariant = TCM::Grapple(getComponentId());
		}

		Body body;
		float influenceRadius;
		TextureComponentVariant texture;
		ComponentId renderingSetup;
		std::optional<Shaders::ProgramId> customShadersProgram;
		ResolutionMode resolutionMode = ResolutionMode::Normal;

		glm::vec2 previousCenter{ 0.0f, 0.0f };

		GLenum drawMode = GL_TRIANGLES;
		GLenum bufferDataUsage = GL_STATIC_DRAW;

		bool preserveTextureRatio = false;

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
	};
}
