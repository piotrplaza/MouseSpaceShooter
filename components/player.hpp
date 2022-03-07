#pragma once

#include "componentBase.hpp"
#include "typeComponentMappers.hpp"

#include <tools/b2Helpers.hpp>
#include <tools/graphicsHelpers.hpp>

#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/bodyUserData.hpp>

#include <ogl/shaders.hpp>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <vector>
#include <optional>
#include <functional>
#include <variant>

namespace Components
{
	struct Player : ComponentBase
	{
		using ComponentBase::ComponentBase;

		Player(Body body,
			TextureComponentVariant texture = std::monostate{},
			std::vector<glm::vec2> texCoord = {},
			ComponentId renderingSetup = 0,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			body(std::move(body)),
			texture(texture),
			texCoord(texCoord),
			renderingSetup(renderingSetup),
			customShadersProgram(customShadersProgram)
		{
			Tools::AccessUserData(*this->body).bodyComponentVariant = TCM::Player(getComponentId());
		}

		Body body;
		TextureComponentVariant texture;
		ComponentId renderingSetup;
		std::optional<Shaders::ProgramId> customShadersProgram;
		std::vector<glm::vec2> texCoord;
		ResolutionMode resolutionMode = ResolutionMode::Normal;

		bool connectIfApproaching = false;
		float autoRotationFactor = 0.5f;

		std::unique_ptr<b2Joint, b2JointDeleter> grappleJoint;
		ComponentId connectedGrappleId = 0;
		ComponentId weakConnectedGrappleId = 0;
		glm::vec2 previousCenter{ 0.0f, 0.0f };

		bool throttling = false;

		GLenum drawMode = GL_TRIANGLES;
		GLenum bufferDataUsage = GL_DYNAMIC_DRAW;
		
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
			if (texCoord.empty())
			{
				const auto positions = getPositions();
				return std::vector<glm::vec2>(positions.begin(), positions.end());
			}
			else
			{
				const auto positions = getPositions();
				if (texCoord.size() < positions.size())
				{
					std::vector<glm::vec2> cyclicTexCoord;
					cyclicTexCoord.reserve(positions.size());
					for (size_t i = 0; i < positions.size(); ++i)
						cyclicTexCoord.push_back(texCoord[i % texCoord.size()]);
					return cyclicTexCoord;
				}
				else
				{
					assert(texCoord.size() == positions.size());
					return texCoord;
				}
			}
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
