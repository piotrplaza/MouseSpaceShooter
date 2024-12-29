#pragma once

#include "_physical.hpp"

#include <tools/b2Helpers.hpp>

#include <globals/collisionBits.hpp>

#include <glm/glm.hpp>

#include <optional>
#include <limits>

namespace Components
{
	struct Plane : Physical
	{
		Plane(Body body,
			AbstractTextureComponentVariant texture = std::monostate{},
			std::vector<glm::vec2> texCoord = {},
			RenderingSetupF renderingSetupF = nullptr,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<ShadersUtils::ProgramId> customShadersProgram = std::nullopt):
			Physical(std::move(body), texture, std::move(renderingSetupF), renderLayer, customShadersProgram)
		{
		}

		bool connectIfApproaching = false;

		float manoeuvrability = 1.0f;

		struct Controls
		{
			glm::vec2 turningDelta{ 0.0f, 0.0f };
			bool autoRotation = false;
			float autoRotationFactor = 1.0f;
			float throttling = 0.0f;
			bool magneticHook = false;
			bool startPressed = false;
		} controls;

		struct
		{
			glm::vec2 previousCenter{ 0.0f, 0.0f };
			std::unique_ptr<b2Joint, b2JointDeleter> grappleJoint;
			std::optional<ComponentId> connectedGrappleId;
			std::optional<ComponentId> weakConnectedGrappleId;
			float throttleForce = 0.0f;
		} details;

		void init(ComponentId id, bool static_) override
		{
			ComponentBase::init(id, static_);
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::actor, Globals::CollisionBits::all);
			setBodyComponentVariant(CM::Plane(*this));
		}

		void throttle(float forceFactor)
		{
			const float angle = body->GetAngle();
			const glm::vec2 planeDirection(glm::cos(angle), glm::sin(angle));

			details.throttleForce = [&]()
			{
				glm::vec2 velocityDirection(ToVec2<glm::vec2>(body->GetLinearVelocity()));
				if (velocityDirection == glm::vec2(0.0f))
					return forceFactor;

				const float forceModifier = 1.0f + (1.0f - glm::dot(planeDirection, glm::normalize(velocityDirection))) / 2.0f * manoeuvrability;
				return forceFactor * forceModifier;
			}();

			body->ApplyForce(ToVec2<b2Vec2>(planeDirection * details.throttleForce), body->GetWorldCenter(), true);
		}

		glm::vec2 getHorizontalOffsets() const
		{
			glm::vec2 results(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest());
			for (const auto& vertex : Tools::GetVertices(*body))
			{
				results[0] = std::min(results[0], vertex.x);
				results[1] = std::max(results[1], vertex.x);
			}
			return results;
		}

		glm::vec2 getVerticalOffsets() const
		{
			glm::vec2 results(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest());
			for (const auto& vertex : Tools::GetVertices(*body))
			{
				results[0] = std::min(results[0], vertex.y);
				results[1] = std::max(results[1], vertex.y);
			}
			return results;
		}
	};
}
