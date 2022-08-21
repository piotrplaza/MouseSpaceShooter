#pragma once

#include "_componentBase.hpp"
#include "_physical.hpp"

#include <tools/b2Helpers.hpp>

#include <glm/glm.hpp>

#include <optional>

namespace Components
{
	struct Plane : ComponentBase, Physical
	{
		Plane() = default;

		Plane(Body body,
			TextureComponentVariant texture = std::monostate{},
			std::vector<glm::vec2> texCoord = {},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			Physical(std::move(body), TCM::Plane(getComponentId()), texture, renderingSetup, renderLayer, customShadersProgram)
		{
		}

		std::function<void()> step;

		bool connectIfApproaching = false;

		float manoeuvrability = 1.0f;

		struct
		{
			glm::vec2 turningDelta{ 0.0f, 0.0f };
			bool autoRotation = false;
			float autoRotationFactor = 1.0f;
			float throttling = 0.0f;
			bool magneticHook = false;
		} controls;

		struct
		{
			glm::vec2 previousCenter{ 0.0f, 0.0f };
			std::unique_ptr<b2Joint, b2JointDeleter> grappleJoint;
			std::optional<ComponentId> connectedGrappleId;
			std::optional<ComponentId> weakConnectedGrappleId;
			float throttleForce = 0.0f;
		} details;

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
	};
}
