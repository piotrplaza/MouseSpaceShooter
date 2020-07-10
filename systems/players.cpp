#include "players.hpp"

#include <vector>
#include <limits>

#include <GL/glew.h>

#include <Box2D/Box2D.h>

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>

#include <tools/b2Helpers.hpp>

#include <globals.hpp>

#include <components/player.hpp>
#include <components/physics.hpp>
#include <components/mvp.hpp>
#include <components/mouseState.hpp>
#include <components/grapple.hpp>
#include <components/connection.hpp>
#include <components/texture.hpp>
#include <components/textureDef.hpp>

namespace Systems
{
	Players::Players() : player(Globals::Components::players.front())
	{
		initGraphics();
	}

	void Players::initGraphics()
	{
		basicShadersProgram = std::make_unique<Shaders::Programs::Basic>();
		sceneCoordTexturedShadersProgram = std::make_unique<Shaders::Programs::SceneCoordTextured>();
		coloredShadersProgram = std::make_unique<Shaders::Programs::Colored>();

		simplePlayersBuffers = std::make_unique<PlayersBuffers>();
		connectionsBuffers = std::make_unique<ConnectionsBuffers>();
		
		updatePlayersPositionsBuffers();
	}

	void Players::updatePlayersPositionsBuffers()
	{
		Tools::UpdateSimpleAndTexturesPositionsBuffers(Globals::Components::players,
			*simplePlayersBuffers, texturesToPlayersBuffers, GL_STATIC_DRAW);
	}

	void Players::updateConnectionsGraphicsBuffers()
	{
		connectionsBuffers->positionsCache.clear();
		connectionsBuffers->colorsCache.clear();

		for (auto& connection : Globals::Components::connections)
		{
			const auto positionsCache = connection.generatePositionsCache();
			connectionsBuffers->positionsCache.insert(connectionsBuffers->positionsCache.end(), positionsCache.begin(), positionsCache.end());

			const auto colorsCache = connection.generateColorsCache();
			connectionsBuffers->colorsCache.insert(connectionsBuffers->colorsCache.end(), colorsCache.begin(), colorsCache.end());
		}

		if (connectionsBuffers->numOfAllocatedVertices < connectionsBuffers->positionsCache.size())
		{
			glBindBuffer(GL_ARRAY_BUFFER, connectionsBuffers->positionBuffer);
			glBufferData(GL_ARRAY_BUFFER, connectionsBuffers->positionsCache.size() * sizeof(connectionsBuffers->positionsCache.front()),
				connectionsBuffers->positionsCache.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, connectionsBuffers->colorBuffer);
			glBufferData(GL_ARRAY_BUFFER, connectionsBuffers->colorsCache.size() * sizeof(connectionsBuffers->colorsCache.front()),
				connectionsBuffers->colorsCache.data(), GL_DYNAMIC_DRAW);
			connectionsBuffers->numOfAllocatedVertices = connectionsBuffers->positionsCache.size();
		}
		else
		{
			glBindBuffer(GL_ARRAY_BUFFER, connectionsBuffers->positionBuffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, connectionsBuffers->positionsCache.size() * sizeof(connectionsBuffers->positionsCache.front()),
				connectionsBuffers->positionsCache.data());
			glBindBuffer(GL_ARRAY_BUFFER, connectionsBuffers->colorBuffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, connectionsBuffers->colorsCache.size() * sizeof(connectionsBuffers->colorsCache.front()),
				connectionsBuffers->colorsCache.data());
		}

		assert(connectionsBuffers->positionsCache.size() == connectionsBuffers->colorsCache.size());
	}

	void Players::step()
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

		updateConnectionsGraphicsBuffers();

		playerPreviousPosition = player.getPosition();
	}

	void Players::render() const
	{
		basicRender();
		sceneCoordTexturedRender();
		coloredRender();
	}

	void Players::basicRender() const
	{
		glUseProgram_proxy(basicShadersProgram->program);
		basicShadersProgram->mvpUniform.setValue(Globals::Components::mvp.getMVP(player.getModelMatrix()));
		basicShadersProgram->colorUniform.setValue({ 1.0f, 1.0f, 1.0f, 1.0f });
		glBindVertexArray(simplePlayersBuffers->positionBuffer);
		glDrawArrays(GL_TRIANGLES, 0, simplePlayersBuffers->positionsCache.size());
	}

	void Players::sceneCoordTexturedRender() const
	{
		using namespace Globals::Components;

		glUseProgram_proxy(sceneCoordTexturedShadersProgram->program);
		sceneCoordTexturedShadersProgram->mvpUniform.setValue(mvp.getMVP(player.getModelMatrix()));

		for (const auto& [texture, texturedPlayerBuffers] : texturesToPlayersBuffers)
		{
			const auto& textureComponent = textures[texture];
			const auto& textureDefComponent = texturesDef[texture];

			sceneCoordTexturedShadersProgram->texture1Uniform.setValue(texture);
			sceneCoordTexturedShadersProgram->textureTranslateUniform.setValue(textureDefComponent.translate);
			sceneCoordTexturedShadersProgram->textureScaleUniform.setValue(
				{ (float)textureComponent.height / textureComponent.width * textureDefComponent.scale.x, textureDefComponent.scale.y });
			glBindVertexArray(texturedPlayerBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, texturedPlayerBuffers.positionsCache.size());
		}
	}

	void Players::coloredRender() const
	{
		glUseProgram_proxy(coloredShadersProgram->program);
		coloredShadersProgram->mvpUniform.setValue(Globals::Components::mvp.getVP());
		glBindVertexArray(connectionsBuffers->vertexArray);
		glDrawArrays(GL_LINES, 0, connectionsBuffers->positionsCache.size());
	}

	void Players::turn(glm::vec2 controllerDelta) const
	{
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
				const float playerSideAngle = playerAngle + glm::half_pi<float>() *
					(Globals::Components::mouseState.mmb ? -1.0f : 1.0f);
				const glm::vec2 playerDirection = { std::cos(playerSideAngle), std::sin(playerSideAngle) };
				const float velocityDot = glm::dot(playerDirection, normalizedStepVelocity);

				player.body->SetTransform(player.body->GetPosition(), playerAngle + velocityDot *
					stepVelocityLength * Globals::Constants::playerAutoRotationFactor);
			}
		}
	}

	void Players::throttle(bool active) const
	{
		if (!active) return;

		const float currentAngle = player.body->GetAngle();
		player.body->ApplyForce(b2Vec2(glm::cos(currentAngle),
			glm::sin(currentAngle)) * Globals::Constants::playerForwardForce, player.body->GetWorldCenter(), true);
	}

	void Players::magneticHook(bool active) const
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

	void Players::createGrappleJoint() const
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

	Players::PlayersBuffers::PlayersBuffers()
	{
		glCreateVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &positionBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
	}

	Players::PlayersBuffers::~PlayersBuffers()
	{
		glDeleteBuffers(1, &positionBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}

	Players::ConnectionsBuffers::ConnectionsBuffers()
	{
		glCreateVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &positionBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
		glGenBuffers(1, &colorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(1);
	}

	Players::ConnectionsBuffers::~ConnectionsBuffers()
	{
		glDeleteBuffers(1, &positionBuffer);
		glDeleteBuffers(1, &colorBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}
}
