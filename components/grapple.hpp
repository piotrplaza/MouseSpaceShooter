#pragma once

#include <vector>
#include <optional>
#include <functional>

#include <glm/glm.hpp>

#include <Box2D/Box2D.h>

#include <ogl/shaders.hpp>

#include <tools/graphicsHelpers.hpp>

namespace Components
{
	struct Grapple
	{
		Grapple(std::unique_ptr<b2Body, b2BodyDeleter> body, float influenceRadius, std::optional<unsigned> texture = std::nullopt,
			std::function<std::function<void()>(Shaders::ProgramId)> renderingSetup = nullptr,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			body(std::move(body)),
			influenceRadius(influenceRadius),
			texture(texture),
			renderingSetup(std::move(renderingSetup)),
			customShadersProgram(customShadersProgram)
		{
		}

		std::unique_ptr<b2Body, b2BodyDeleter> body;
		float influenceRadius;
		std::optional<unsigned> texture;
		std::function<std::function<void()>(Shaders::ProgramId)> renderingSetup;
		std::optional<Shaders::ProgramId> customShadersProgram;

		glm::vec2 previousCenter{ 0.0f, 0.0f };

		glm::vec2 getCenter() const
		{
			return ToVec2<glm::vec2>(body->GetWorldCenter());
		}

		std::vector<glm::vec3> getPositions() const
		{
			using namespace Globals::Constants;

			const auto& fixture = *body->GetFixtureList();
			assert(!fixture.GetNext());
			assert(fixture.GetType() == b2Shape::e_circle); //Temporary. TODO: Add other shapes.
			const auto& circleShape = static_cast<const b2CircleShape&>(*fixture.GetShape());

			return Tools::CreatePositionsOfCircle(ToVec2<glm::vec2>(circleShape.m_p), circleShape.m_radius, circleGraphicsComplexity);
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
