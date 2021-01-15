#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <functional>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <componentBase.hpp>

#include <ogl/shaders.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/graphicsHelpers.hpp>

#include <bodyUserData.hpp>

namespace Components
{
	struct Player : ComponentBase
	{
		using RenderingSetup = std::function<std::function<void()>(Shaders::ProgramId)>;

		Player(std::unique_ptr<b2Body, b2BodyDeleter> body, std::optional<unsigned> texture = std::nullopt,
			std::unique_ptr<RenderingSetup> renderingSetup = nullptr,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			body(std::move(body)),
			texture(texture),
			renderingSetup(std::move(renderingSetup)),
			customShadersProgram(customShadersProgram)
		{
			Tools::AccessUserData(*this->body).componentId = componentId;
		}

		std::unique_ptr<b2Body, b2BodyDeleter> body;
		std::optional<unsigned> texture;
		std::unique_ptr<RenderingSetup> renderingSetup;
		std::optional<Shaders::ProgramId> customShadersProgram;
		bool connectIfApproaching = false;
		float autoRotationFactor = 0.5f;

		std::unique_ptr<b2Joint, b2JointDeleter> grappleJoint;
		int connectedGrappleId = -1;
		int weakConnectedGrappleId = -1;
		glm::vec2 previousCenter{ 0.0f, 0.0f };

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

		float getAngle() const
		{
			return body->GetAngle();
		}

		glm::vec2 getVelocity() const
		{
			return ToVec2<glm::vec2>(body->GetLinearVelocity());
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

		bool isTextureRatioPreserved() const
		{
			return true;
		}
	};
}
