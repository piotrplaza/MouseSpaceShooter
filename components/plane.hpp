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
	struct Plane : ComponentBase
	{
		using ComponentBase::ComponentBase;

		Plane(Body body,
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
			Tools::AccessUserData(*this->body).bodyComponentVariant = TCM::Plane(getComponentId());
		}

		struct
		{
			glm::vec2 turningDelta{ 0.0f, 0.0f };
			bool autoRotation = false;
			float autoRotationFactor = 0.5f;
			bool throttling = false;
			bool magneticHook = false;
		} controls;

		Body body;
		TextureComponentVariant texture;
		ComponentId renderingSetup;
		std::optional<Shaders::ProgramId> customShadersProgram;
		std::vector<glm::vec2> texCoord;
		ResolutionMode resolutionMode = ResolutionMode::Normal;

		bool connectIfApproaching = false;

		std::unique_ptr<b2Joint, b2JointDeleter> grappleJoint;
		ComponentId connectedGrappleId = 0;
		ComponentId weakConnectedGrappleId = 0;

		GLenum drawMode = GL_TRIANGLES;
		GLenum bufferDataUsage = GL_DYNAMIC_DRAW;

		bool preserveTextureRatio = false;

		struct
		{
			glm::vec2 previousCenter{ 0.0f, 0.0f };
		} details;
		
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
			if (texCoord.empty())
			{
				const auto vertices = getVertices();
				return std::vector<glm::vec2>(vertices.begin(), vertices.end());
			}
			else
			{
				const auto vertices = getVertices();
				if (texCoord.size() < vertices.size())
				{
					std::vector<glm::vec2> cyclicTexCoord;
					cyclicTexCoord.reserve(vertices.size());
					for (size_t i = 0; i < vertices.size(); ++i)
						cyclicTexCoord.push_back(texCoord[i % texCoord.size()]);
					return cyclicTexCoord;
				}
				else
				{
					assert(texCoord.size() == vertices.size());
					return texCoord;
				}
			}
		}

		glm::mat4 getModelMatrix() const
		{
			return Tools::GetModelMatrix(*body);
		}
	};
}
