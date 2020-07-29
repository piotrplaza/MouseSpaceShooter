#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <functional>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ogl/shaders.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/graphicsHelpers.hpp>

namespace Components
{
	struct Player
	{
		Player()
		{
		}

		Player(std::unique_ptr<b2Body, b2BodyDeleter> body, std::optional<unsigned> texture = std::nullopt,
			std::function<std::function<void()>(Shaders::ProgramId)> renderingSetup = nullptr):
			body(std::move(body)),
			texture(texture),
			renderingSetup(renderingSetup)
		{
		}

		std::unique_ptr<b2Body, b2BodyDeleter> body;
		std::optional<unsigned> texture;
		std::function<std::function<void()>(Shaders::ProgramId)> renderingSetup;

		std::unique_ptr<b2Joint, b2JointDeleter> grappleJoint;
		int connectedGrappleId = -1;
		int weakConnectedGrappleId = -1;

		bool throttling = false;
		
		void setPosition(const glm::vec2& position)
		{
			body->SetTransform({ position.x, position.y }, body->GetAngle());
		}

		void setRotation(float angle)
		{
			body->SetTransform(body->GetPosition(), angle);
		}

		void resetKinematic()
		{
			body->SetLinearVelocity({ 0.0f, 0.0f });
			body->SetAngularVelocity(0.0f);
		}

		glm::vec2 getCenter() const
		{
			return ToVec2<glm::vec2>(body->GetWorldCenter());
		}

		glm::vec2 getVelocity() const
		{
			return ToVec2<glm::vec2>(body->GetLinearVelocity());
		}

		std::vector<glm::vec3> getPositions() const
		{
			std::vector<glm::vec3> positions;

			const auto& fixture = *body->GetFixtureList();
			assert(!fixture.GetNext());
			assert(fixture.GetType() == b2Shape::e_polygon);
			const auto& polygonShape = static_cast<const b2PolygonShape&>(*fixture.GetShape());
			positions.reserve(polygonShape.m_count);
			for (int i = 0; i < polygonShape.m_count; ++i)
			{
				const auto& b2v = polygonShape.m_vertices[i];
				positions.emplace_back(b2v.x, b2v.y, 0.0f);
			}

			return positions;
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
