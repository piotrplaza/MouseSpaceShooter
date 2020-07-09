#include "player.hpp"

#include <vector>
#include <limits>

#include <Box2D/Box2D.h>

#include <ogl/oglProxy.hpp>

#include <tools/b2Helpers.hpp>

#include <globals.hpp>

#include <components/player.hpp>
#include <components/physics.hpp>
#include <components/mvp.hpp>
#include <components/mouseState.hpp>
#include <components/grapple.hpp>
#include <components/connection.hpp>

namespace Systems
{
	Player::Player()
	{
		initGraphics();
	}

	void Player::initGraphics()
	{
		using namespace Globals::Components;

		basicShadersProgram = std::make_unique<Shaders::Programs::Basic>();
		coloredShadersProgram = std::make_unique<Shaders::Programs::Colored>();

		playerBuffers = std::make_unique<PlayerBuffers>();
		connectionsBuffers = std::make_unique<ConnectionsBuffers>();
		
		updateStaticPlayerGraphics();
	}

	void Player::updateStaticPlayerGraphics() const
	{
		using namespace Globals::Components;

		playerBuffers->verticesCache.clear();
		const auto verticesCache = player.generateVerticesCache();
		playerBuffers->verticesCache.insert(playerBuffers->verticesCache.end(), verticesCache.begin(), verticesCache.end());

		glBindBuffer(GL_ARRAY_BUFFER, playerBuffers->vertexBuffer);
		if (playerBuffers->vertexBufferAllocation < playerBuffers->verticesCache.size())
		{
			glBufferData(GL_ARRAY_BUFFER, playerBuffers->verticesCache.size() * sizeof(playerBuffers->verticesCache.front()), playerBuffers->verticesCache.data(), GL_STATIC_DRAW);
			playerBuffers->vertexBufferAllocation = playerBuffers->verticesCache.size();
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, playerBuffers->verticesCache.size() * sizeof(playerBuffers->verticesCache.front()), playerBuffers->verticesCache.data());
		}
	}

	void Player::updateConnectionsGraphics()
	{
		using namespace Globals::Components;

		connectionsBuffers->verticesCache.clear();
		connectionsBuffers->colorsCache.clear();
		for (auto& connection : connections)
		{
			const auto verticesCache = connection.generateVerticesCache();
			connectionsBuffers->verticesCache.insert(connectionsBuffers->verticesCache.end(), verticesCache.begin(), verticesCache.end());

			const auto colorsCache = connection.generateColorsCache();
			connectionsBuffers->colorsCache.insert(connectionsBuffers->colorsCache.end(), colorsCache.begin(), colorsCache.end());
		}

		if (connectionsBuffers->vertexBufferAllocation < connectionsBuffers->verticesCache.size())
		{
			glBindBuffer(GL_ARRAY_BUFFER, connectionsBuffers->vertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, connectionsBuffers->verticesCache.size() * sizeof(connectionsBuffers->verticesCache.front()),
				connectionsBuffers->verticesCache.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, connectionsBuffers->colorBuffer);
			glBufferData(GL_ARRAY_BUFFER, connectionsBuffers->colorsCache.size() * sizeof(connectionsBuffers->colorsCache.front()),
				connectionsBuffers->colorsCache.data(), GL_DYNAMIC_DRAW);
			connectionsBuffers->vertexBufferAllocation = connectionsBuffers->verticesCache.size();
		}
		else
		{
			glBindBuffer(GL_ARRAY_BUFFER, connectionsBuffers->vertexBuffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, connectionsBuffers->verticesCache.size() * sizeof(connectionsBuffers->verticesCache.front()),
				connectionsBuffers->verticesCache.data());
			glBindBuffer(GL_ARRAY_BUFFER, connectionsBuffers->colorBuffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, connectionsBuffers->colorsCache.size() * sizeof(connectionsBuffers->colorsCache.front()),
				connectionsBuffers->colorsCache.data());
		}
	}

	void Player::step()
	{
		using namespace Globals::Components;

		const glm::ivec2 windowSpaceMouseDelta = mouseState.getMouseDelta();
		const glm::vec2 mouseDelta = { windowSpaceMouseDelta.x, -windowSpaceMouseDelta.y };

		if (firstStep)
		{
			playerPreviousPosition = player.getPosition();
			firstStep = false;
		}

		turn(mouseDelta);
		throttle(mouseState.rmb);
		magneticHook(mouseState.lmb);

		updateConnectionsGraphics();

		playerPreviousPosition = player.getPosition();
	}

	void Player::render() const
	{
		using namespace Globals::Components;

		glUseProgram_proxy(basicShadersProgram->program);
		basicShadersProgram->mvpUniform.setValue(Globals::Components::mvp.getMVP(player.getModelMatrix()));
		basicShadersProgram->colorUniform.setValue({ 1.0f, 1.0f, 1.0f, 1.0f });
		glBindVertexArray(playerBuffers->vertexBuffer);
		glDrawArrays(GL_TRIANGLES, 0, playerBuffers->verticesCache.size());

		glUseProgram_proxy(coloredShadersProgram->program);
		coloredShadersProgram->mvpUniform.setValue(Globals::Components::mvp.getVP());
		glBindVertexArray(connectionsBuffers->vertexArray);
		glDrawArrays(GL_LINES, 0, connectionsBuffers->verticesCache.size());
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
			const glm::vec2 stepVelocity = player.getPosition() - playerPreviousPosition;
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
					glm::distance(playerPreviousPosition, grapple.getPosition()))
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

	Player::PlayerBuffers::PlayerBuffers()
	{
		glCreateVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
	}

	Player::PlayerBuffers::~PlayerBuffers()
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}

	Player::ConnectionsBuffers::ConnectionsBuffers()
	{
		glCreateVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
		glGenBuffers(1, &colorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(1);
	}

	Player::ConnectionsBuffers::~ConnectionsBuffers()
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteBuffers(1, &colorBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}
}
