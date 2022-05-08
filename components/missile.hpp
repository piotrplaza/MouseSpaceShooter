#pragma once

#include "componentBase.hpp"

#include <commonTypes/resolutionMode.hpp>

#include <ogl/shaders.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/b2Helpers.hpp>

#include <commonTypes/bodyUserData.hpp>

#include <Box2D/Box2D.h>

#include <glm/glm.hpp>

#include <vector>
#include <optional>
#include <functional>

namespace Components
{
	struct Missile : ComponentBase
	{
		using ComponentBase::ComponentBase;

		Missile(Body body,
			TextureComponentVariant texture = std::monostate{},
			ComponentId renderingSetup = 0,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			body(std::move(body)),
			texture(texture),
			renderingSetup(renderingSetup),
			customShadersProgram(customShadersProgram)
		{
			Tools::AccessUserData(*this->body).bodyComponentVariant = TCM::Missile(getComponentId());
		}

		Body body;
		TextureComponentVariant texture;
		ComponentId renderingSetup;
		std::optional<Shaders::ProgramId> customShadersProgram;
		std::function<void()> step;
		ResolutionMode resolutionMode = ResolutionMode::Normal;

		static constexpr GLenum drawMode = GL_TRIANGLES;
		static constexpr GLenum bufferDataUsage = GL_STATIC_DRAW;

		bool preserveTextureRatio = false;

		glm::vec2 getCenter() const
		{
			return ToVec2<glm::vec2>(body->GetWorldCenter());
		}

		std::vector<glm::vec3> getVertexPositions() const
		{
			return Tools::GetVertices(*body);
		}

		std::vector<glm::vec3> getTransformedVertexPositions() const
		{
			return Tools::Transform(getVertexPositions(), getModelMatrix());
		}

		const std::vector<glm::vec2> getTexCoord() const
		{
			const auto positions = getVertexPositions();
			return std::vector<glm::vec2>(positions.begin(), positions.end());
		}

		glm::mat4 getModelMatrix() const
		{
			return Tools::GetModelMatrix(*body);
		}
	};
}
