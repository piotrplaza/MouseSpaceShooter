#include "players.hpp"

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
		using namespace Globals::Components;

		basicShadersProgram = std::make_unique<Shaders::Programs::Basic>();
		sceneCoordTexturedShadersProgram = std::make_unique<Shaders::Programs::SceneCoordTextured>();
		coloredShadersProgram = std::make_unique<Shaders::Programs::Colored>();

		simplePlayersBuffers = std::make_unique<PlayerBuffers>();
		connectionsBuffers = std::make_unique<ConnectionsBuffers>();
		
		updatePlayerGraphics();
	}

	void Players::updatePlayerGraphics()
	{
		using namespace Globals::Components;

		for (auto& player : players )
		{
			const auto verticesCache = player.generateVerticesCache();
			if (player.texture)
			{
				auto& texturedPlayerBuffers = texturesToPlayersBuffers[*player.texture];
				texturedPlayerBuffers.verticesCache.insert(texturedPlayerBuffers.verticesCache.end(), verticesCache.begin(), verticesCache.end());
			}
			else
			{
				simplePlayersBuffers->verticesCache.insert(simplePlayersBuffers->verticesCache.end(), verticesCache.begin(), verticesCache.end());
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, simplePlayersBuffers->vertexBuffer);
		if (simplePlayersBuffers->vertexBufferAllocation < simplePlayersBuffers->verticesCache.size())
		{
			glBufferData(GL_ARRAY_BUFFER, simplePlayersBuffers->verticesCache.size() * sizeof(simplePlayersBuffers->verticesCache.front()),
				simplePlayersBuffers->verticesCache.data(), GL_STATIC_DRAW);
			simplePlayersBuffers->vertexBufferAllocation = simplePlayersBuffers->verticesCache.size();
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, simplePlayersBuffers->verticesCache.size() * sizeof(simplePlayersBuffers->verticesCache.front()),
				simplePlayersBuffers->verticesCache.data());
		}

		for (auto& [texture, texturedPlayerBuffers] : texturesToPlayersBuffers)
		{
			glBindBuffer(GL_ARRAY_BUFFER, texturedPlayerBuffers.vertexBuffer);
			if (texturedPlayerBuffers.vertexBufferAllocation < texturedPlayerBuffers.verticesCache.size())
			{
				glBufferData(GL_ARRAY_BUFFER, texturedPlayerBuffers.verticesCache.size() * sizeof(texturedPlayerBuffers.verticesCache.front()),
					texturedPlayerBuffers.verticesCache.data(), GL_STATIC_DRAW);
				texturedPlayerBuffers.vertexBufferAllocation = texturedPlayerBuffers.verticesCache.size();
			}
			else
			{
				glBufferSubData(GL_ARRAY_BUFFER, 0, texturedPlayerBuffers.verticesCache.size() * sizeof(texturedPlayerBuffers.verticesCache.front()),
					texturedPlayerBuffers.verticesCache.data());
			}
		}
	}

	void Players::updateConnectionsGraphics()
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

		updateConnectionsGraphics();

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
		using namespace Globals::Components;

		glUseProgram_proxy(basicShadersProgram->program);
		basicShadersProgram->mvpUniform.setValue(mvp.getMVP(player.getModelMatrix()));
		basicShadersProgram->colorUniform.setValue({ 1.0f, 1.0f, 1.0f, 1.0f });
		glBindVertexArray(simplePlayersBuffers->vertexBuffer);
		glDrawArrays(GL_TRIANGLES, 0, simplePlayersBuffers->verticesCache.size());
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
			glDrawArrays(GL_TRIANGLES, 0, texturedPlayerBuffers.verticesCache.size());
		}
	}

	void Players::coloredRender() const
	{
		using namespace Globals::Components;

		glUseProgram_proxy(coloredShadersProgram->program);
		coloredShadersProgram->mvpUniform.setValue(mvp.getVP());
		glBindVertexArray(connectionsBuffers->vertexArray);
		glDrawArrays(GL_LINES, 0, connectionsBuffers->verticesCache.size());
	}

	void Players::turn(glm::vec2 controllerDelta) const
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

	void Players::throttle(bool active) const
	{
		using namespace Globals::Components;
		using namespace Globals::Constants;

		if (!active) return;

		const float currentAngle = player.body->GetAngle();
		player.body->ApplyForce(b2Vec2(glm::cos(currentAngle),
			glm::sin(currentAngle)) * playerForwardForce, player.body->GetWorldCenter(), true);
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

	Players::PlayerBuffers::PlayerBuffers()
	{
		glCreateVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
	}

	Players::PlayerBuffers::~PlayerBuffers()
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}

	Players::ConnectionsBuffers::ConnectionsBuffers()
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

	Players::ConnectionsBuffers::~ConnectionsBuffers()
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteBuffers(1, &colorBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}
}
