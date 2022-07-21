#pragma once

#include "_componentBase.hpp"
#include "_renderable.hpp"

#include <tools/b2Helpers.hpp>

#include <commonTypes/bodyUserData.hpp>

namespace Components
{
	struct Plane : ComponentBase, Renderable
	{
		Plane() = default;

		Plane(Body body,
			TextureComponentVariant texture = std::monostate{},
			std::vector<glm::vec2> texCoord = {},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			Renderable(texture, renderingSetup, renderLayer, customShadersProgram),
			body(std::move(body))
		{
			Tools::AccessUserData(*this->body).bodyComponentVariant = TCM::Plane(getComponentId());
		}

		Body body;

		bool connectIfApproaching = false;

		struct
		{
			glm::vec2 turningDelta{ 0.0f, 0.0f };
			bool autoRotation = false;
			float autoRotationFactor = 0.5f;
			bool throttling = false;
			bool magneticHook = false;
		} controls;

		struct
		{
			glm::vec2 previousCenter{ 0.0f, 0.0f };
			std::unique_ptr<b2Joint, b2JointDeleter> grappleJoint;
			ComponentId connectedGrappleId = 0;
			ComponentId weakConnectedGrappleId = 0;
		} details;

		std::vector<glm::vec3> getVertices(bool transformed = false) const override
		{
			return transformed
				? Tools::Transform(Tools::GetVertices(*body), getModelMatrix())
				: Tools::GetVertices(*body);
		}

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

		float getAngle() const
		{
			return body->GetAngle();
		}

		glm::vec2 getVelocity() const
		{
			return ToVec2<glm::vec2>(body->GetLinearVelocity());
		}

		glm::mat4 getModelMatrix() const override
		{
			return Tools::GetModelMatrix(*body);
		}
	};
}
