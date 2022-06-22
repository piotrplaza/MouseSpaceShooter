#pragma once

#include "componentBase.hpp"
#include "typeComponentMappers.hpp"

#include <commonTypes/resolutionMode.hpp>

#include "decoration.hpp"

#include <tools/graphicsHelpers.hpp>
#include <tools/b2Helpers.hpp>

#include <commonTypes/bodyUserData.hpp>

#include <ogl/shaders.hpp>

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

		GLenum drawMode = GL_TRIANGLES;
		GLenum bufferDataUsage = GL_STATIC_DRAW;

		bool preserveTextureRatio = false;

		bool render = true;

		std::vector<DecorationDef> subsequence;
		unsigned posInSubsequence = 0;

		glm::vec2 getCenter() const
		{
			return ToVec2<glm::vec2>(body->GetWorldCenter());
		}

		std::vector<glm::vec3> getVertices() const
		{
			return Tools::GetVertices(*body);
		}

		std::vector<glm::vec3> getTransformedVertices() const
		{
			return Tools::Transform(getVertices(), getModelMatrix());
		}

		const std::vector<glm::vec2> getTexCoord() const
		{
			const auto vertices = getVertices();
			return std::vector<glm::vec2>(vertices.begin(), vertices.end());
		}

		glm::mat4 getModelMatrix() const
		{
			return Tools::GetModelMatrix(*body);
		}
	};
}
