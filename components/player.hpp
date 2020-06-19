#pragma once

#include <memory>
#include <array>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <tools/b2Helpers.hpp>

namespace Components
{
	struct Player
	{
		std::unique_ptr<b2Body, b2BodyDeleter> body;
		std::unique_ptr<b2Joint, b2JointDeleter> grappleJoint;
		std::array<glm::vec3, 3> verticesCache{};
		int connectedGrappleId = -1;

		glm::vec2 previousPosition{0.0f, 0.0f};
		
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

		glm::vec2 getPosition() const
		{
			return { body->GetWorldCenter().x, body->GetWorldCenter().y };
		}

		glm::mat4 getModelMatrix() const
		{
			const auto& transform = body->GetTransform();
			return glm::rotate(glm::translate(glm::mat4(1.0f), { transform.p.x, transform.p.y, 0.0f }),
				transform.q.GetAngle(), {0.0f, 0.0f, 1.0f});
		}

		void updateVerticesCache()
		{
			const auto& fixture = *body->GetFixtureList();
			assert(!fixture.GetNext());
			assert(fixture.GetType() == b2Shape::e_polygon);

			const auto& polygonShape = static_cast<const b2PolygonShape&>(*fixture.GetShape());
			assert(polygonShape.m_count == 3);

			for (int i = 0; i < polygonShape.m_count; ++i)
			{
				const auto& b2v = polygonShape.m_vertices[i];
				verticesCache[i] = { b2v.x, b2v.y, 0.0f };
			}
		}
	};
}
