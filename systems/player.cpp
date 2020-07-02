#include "player.hpp"

#include <vector>
#include <limits>

#include <Box2D/Box2D.h>

#include <glm/gtc/type_ptr.hpp>

#include <globals.hpp>

#include <components/player.hpp>
#include <components/physics.hpp>
#include <components/mvp.hpp>
#include <components/mouseState.hpp>
#include <components/grapple.hpp>
#include <components/connection.hpp>

b2Vec2 operator *(const b2Vec2 v, const float s)
{
	return { v.x * s, v.y * s };
}

namespace Systems
{
	Player::Player()
	{
		initGraphics();
	}

	void Player::initGraphics()
	{
		using namespace Globals::Components;

		basicShadersProgram = shaders::LinkProgram(shaders::CompileShaders("shaders/basic.vs", "shaders/basic.fs"),
			{ {0, "bPos"} });
		basicShadersMVPUniform = glGetUniformLocation(basicShadersProgram, "mvp");
		basicShadersColorUniform = glGetUniformLocation(basicShadersProgram, "color");

		glCreateVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
		
		updateStaticPlayerGraphics();
	}

	void Player::updateStaticPlayerGraphics() const
	{
		using namespace Globals::Components;

		player.updateVerticesCache();
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, player.verticesCache.size() * sizeof(player.verticesCache.front()), player.verticesCache.data(), GL_STATIC_DRAW);
	}

	void Player::step()
	{
		using namespace Globals::Components;

		const glm::ivec2 windowSpaceMouseDelta = mouseState.getMouseDelta();
		const glm::vec2 mouseDelta = { windowSpaceMouseDelta.x, -windowSpaceMouseDelta.y };

		if (firstStep)
		{
			player.previousPosition = player.getPosition();
			firstStep = false;
		}

		turn(mouseDelta);
		throttle(mouseState.rmb);
		magneticHook(mouseState.lmb);

		player.previousPosition = player.getPosition();
	}

	void Player::render() const
	{
		using namespace Globals::Components;

		glUseProgram(basicShadersProgram);
		glUniformMatrix4fv(basicShadersMVPUniform, 1, GL_FALSE,
			glm::value_ptr(Globals::Components::mvp.getMVP(player.getModelMatrix())));

		glUniform4f(basicShadersColorUniform, 1.0f, 1.0f, 1.0f, 1.0f);
		glBindVertexArray(vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, player.verticesCache.size());

		//debugRender();
	}

	void Player::turn(glm::vec2 controllerDelta) const
	{
		using namespace Globals::Components;

		if (glm::length(controllerDelta) > 0)
		{
			const float playerAngle = player.body->GetAngle();
			const float playerSideAngle = playerAngle + glm::half_pi<float>();
			const glm::vec2 playerDirection = { std::cos(playerSideAngle), std::sin(playerSideAngle) };
			const float controllerDot = glm::dot(playerDirection, controllerDelta);

			player.body->SetTransform(player.body->GetPosition(), playerAngle + controllerDot * Globals::Constants::mouseSensitivity);
		}

		if (player.grappleJoint)
		{
			const glm::vec2 stepVelocity = player.getPosition() - player.previousPosition;
			const float stepVelocityLength = glm::length(stepVelocity);

			if (stepVelocityLength > 0.0f)
			{
				const glm::vec2 normalizedStepVelocity = stepVelocity / stepVelocityLength;
				const float playerAngle = player.body->GetAngle();
				const float playerSideAngle = playerAngle + glm::half_pi<float>() * (mouseState.mmb ? -1.0f : 1.0f);
				const glm::vec2 playerDirection = { std::cos(playerSideAngle), std::sin(playerSideAngle) };
				const float velocityDot = glm::dot(playerDirection, normalizedStepVelocity);

				player.body->SetTransform(player.body->GetPosition(), playerAngle + velocityDot *
					stepVelocityLength * Globals::Constants::playerAutoRotationFactor);
			}
		}
	}

	void Player::throttle(bool active) const
	{
		using namespace Globals::Components;
		using namespace Globals::Constants;

		if (!active) return;

		const float currentAngle = player.body->GetAngle();
		player.body->ApplyForce(b2Vec2(glm::cos(currentAngle),
			glm::sin(currentAngle)) * playerForwardForce, player.body->GetWorldCenter(), true);
	}

	void Player::magneticHook(bool active) const
	{
		using namespace Globals::Components;

		connections.clear();

		int nearestGrappleId = -1;
		float nearestGrappleDistance = std::numeric_limits<float>::infinity();
		std::vector<int> grapplesInRange;

		for (int i = 0; i < (int)grapples.size(); ++i)
		{
			const auto& grapple = grapples[i];
			const float grappleDistance = glm::distance(player.getPosition(), grapple.getPosition());

			if (grappleDistance > grapple.influenceRadius) continue;

			grapplesInRange.push_back(i);

			if (grappleDistance < nearestGrappleDistance)
			{
				nearestGrappleDistance = grappleDistance;
				nearestGrappleId = i;
			}
		}

		if (!active)
		{
			player.connectedGrappleId = -1;
			player.weakConnectedGrappleId = -1;
			player.grappleJoint.reset();
		}

		for (auto it = grapplesInRange.begin(); it != grapplesInRange.end(); ++it)
		{
			const auto& grapple = grapples[*it];

			if (*it == nearestGrappleId)
			{
				if (active && !player.grappleJoint &&
					glm::distance(player.getPosition(), grapple.getPosition()) >=
					glm::distance(player.previousPosition, grapple.getPosition()))
				{
					player.connectedGrappleId = *it;
					player.weakConnectedGrappleId = -1;
					createGrappleJoint();
				}
				else if(player.connectedGrappleId != *it)
				{
					if (active)
					{
						if (player.grappleJoint)
						{
							connections.emplace_back(player.getPosition(), grapple.getPosition(),
								glm::vec4(0.0f, 1.0f, 0.0f, 0.2f), 1);
						}
						player.weakConnectedGrappleId = *it;
					}
					else
					{
						connections.emplace_back(player.getPosition(), grapple.getPosition(),
							glm::vec4(0.0f, 1.0f, 0.0f, 0.2f), 1);
					}
				}
			}
			else if (player.connectedGrappleId != *it)
			{
				connections.emplace_back(player.getPosition(), grapple.getPosition(), glm::vec4(1.0f, 0.0f, 0.0f, 0.2f), 1);
			}
		}

		if (player.connectedGrappleId != -1)
		{
			connections.emplace_back(player.getPosition(), grapples[player.connectedGrappleId].getPosition(),
				glm::vec4(0.0f, 0.0f, 1.0f, 0.7f), 20, 0.4f);
		}
		else if (player.weakConnectedGrappleId != -1)
		{
			connections.emplace_back(player.getPosition(), grapples[player.weakConnectedGrappleId].getPosition(),
				glm::vec4(0.0f, 0.0f, 1.0f, 0.5f), 20, 0.1f);
		}
	}

	void Player::createGrappleJoint() const
	{
		using namespace Globals::Components;

		assert(player.connectedGrappleId != -1);

		const auto& grapple = grapples[player.connectedGrappleId];

		b2DistanceJointDef distanceJointDef;
		distanceJointDef.bodyA = player.body.get();
		distanceJointDef.bodyB = grapple.body.get();
		distanceJointDef.localAnchorA = distanceJointDef.bodyA->GetLocalCenter();
		distanceJointDef.localAnchorB = distanceJointDef.bodyB->GetLocalCenter();
		distanceJointDef.length = glm::distance(player.getPosition(), grapple.getPosition());
		distanceJointDef.collideConnected = true;
		player.grappleJoint.reset(physics.world.CreateJoint(&distanceJointDef));
	}

	void Player::debugRender() const
	{
		using namespace Globals::Components;

		glPointSize(10);
		glUniform4f(basicShadersColorUniform, 1.0f, 0.0f, 0.0f, 1.0f);
		glBindVertexArray(0);
		std::vector<glm::vec3> v{
			{glm::vec3(player.getPosition(), 0.0f)}
		};
		glVertexAttribPointer(0, 3, GL_POINTS, false, 0, v.data());
		glDrawArrays(GL_POINTS, 0, v.size());
	}
}
