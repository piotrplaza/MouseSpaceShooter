#pragma once

#include <vector>
#include <optional>
#include <functional>

#include <glm/glm.hpp>

#include <Box2D/Box2D.h>

#include <globals.hpp>
#include <tools/graphicsHelpers.hpp>

namespace Components
{
	struct Grapple
	{
		Grapple(std::unique_ptr<b2Body, b2BodyDeleter> body, float influenceRadius, std::optional<unsigned> texture = std::nullopt,
			std::function<void(Shaders::ProgramId)> renderingSetup = nullptr):
			body(std::move(body)),
			influenceRadius(influenceRadius),
			texture(texture),
			renderingSetup(renderingSetup)
		{
		}

		std::unique_ptr<b2Body, b2BodyDeleter> body;
		float influenceRadius;
		std::optional<unsigned> texture;
		std::function<void(Shaders::ProgramId)> renderingSetup;

		glm::vec2 getPosition() const
		{
			return { body->GetWorldCenter().x, body->GetWorldCenter().y };
		}

		std::vector<glm::vec3> generatePositionsCache(bool transform = true) const
		{
			using namespace Globals::Constants;

			const auto& bodyTransform = body->GetTransform();
			const auto modelMatrix = transform
				? glm::rotate(glm::translate(glm::mat4(1.0f), { bodyTransform.p.x, bodyTransform.p.y, 0.0f }),
					bodyTransform.q.GetAngle(), { 0.0f, 0.0f, 1.0f })
				: glm::mat4(1.0f);
			const auto& fixture = *body->GetFixtureList();
			assert(!fixture.GetNext());
			assert(fixture.GetType() == b2Shape::e_circle); //Temporary. TODO: Add other shapes.
			const auto& circleShape = static_cast<const b2CircleShape&>(*fixture.GetShape());

			return Tools::CreateCirclePositions(ToVec2<glm::vec2>(circleShape.m_p), circleShape.m_radius, circleGraphicsComplexity, modelMatrix);
		}
	};
}
