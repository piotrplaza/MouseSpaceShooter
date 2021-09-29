#include "players.hpp"

#include <vector>
#include <limits>
#include <algorithm>

#include <GL/glew.h>

#include <Box2D/Box2D.h>

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>
#include <ogl/renderingHelpers.hpp>

#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/shaders/colored.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/graphicsHelpers.hpp>

#include <globals.hpp>

#include <components/player.hpp>
#include <components/physics.hpp>
#include <components/mvp.hpp>
#include <components/mouseState.hpp>
#include <components/grapple.hpp>
#include <components/texture.hpp>
#include <components/graphicsSettings.hpp>

namespace
{
	constexpr float playerForwardForce = 10.0f;
	constexpr float mouseSensitivity = 0.01f;
}

namespace Systems
{
	Players::Players() : player(Globals::Components().players().front())
	{
		initGraphics();
	}

	void Players::initGraphics()
	{
		simplePlayersBuffers = std::make_unique<Buffers::PosTexCoordBuffers>();
		connectionsBuffers = std::make_unique<ConnectionsBuffers>();

		updatePlayersTexCoordBuffers();
	}

	void Players::updatePlayersPositionsBuffers()
	{
		Tools::UpdateTransformedPositionsBuffers(Globals::Components().players(),
			*simplePlayersBuffers, texturesToPlayersBuffers, customSimplePlayersBuffers,
			customTexturedPlayersBuffers, customShadersPlayersBuffers);
	}

	void Players::updatePlayersTexCoordBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components().players(), texturesToPlayersBuffers,
			customTexturedPlayersBuffers, customShadersPlayersBuffers);
	}

	void Players::updateConnectionsGraphicsBuffers()
	{
		connectionsBuffers->positionsCache.clear();
		connectionsBuffers->colorsCache.clear();

		for (auto& connection : connections)
		{
			if (connection.segmentsNum > 1)
				connection.segmentsNum = std::max((int)glm::distance(connection.p1, connection.p2) * 2, 2);

			const auto positions = connection.getPositions();
			connectionsBuffers->positionsCache.insert(connectionsBuffers->positionsCache.end(),
				positions.begin(), positions.end());

			const auto colors = connection.getColors();
			connectionsBuffers->colorsCache.insert(connectionsBuffers->colorsCache.end(),
				colors.begin(), colors.end());
		}

		if (connectionsBuffers->numOfAllocatedVertices < connectionsBuffers->positionsCache.size())
		{
			glBindBuffer(GL_ARRAY_BUFFER, connectionsBuffers->positionBuffer);
			glBufferData(GL_ARRAY_BUFFER, connectionsBuffers->positionsCache.size()
				* sizeof(connectionsBuffers->positionsCache.front()),
				connectionsBuffers->positionsCache.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, connectionsBuffers->colorBuffer);
			glBufferData(GL_ARRAY_BUFFER, connectionsBuffers->colorsCache.size()
				* sizeof(connectionsBuffers->colorsCache.front()),
				connectionsBuffers->colorsCache.data(), GL_DYNAMIC_DRAW);
			connectionsBuffers->numOfAllocatedVertices = connectionsBuffers->positionsCache.size();
		}
		else
		{
			glBindBuffer(GL_ARRAY_BUFFER, connectionsBuffers->positionBuffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, connectionsBuffers->positionsCache.size()
				* sizeof(connectionsBuffers->positionsCache.front()),
				connectionsBuffers->positionsCache.data());
			glBindBuffer(GL_ARRAY_BUFFER, connectionsBuffers->colorBuffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, connectionsBuffers->colorsCache.size()
				* sizeof(connectionsBuffers->colorsCache.front()),
				connectionsBuffers->colorsCache.data());
		}

		assert(connectionsBuffers->positionsCache.size() == connectionsBuffers->colorsCache.size());
	}

	void Players::step()
	{
		const glm::ivec2 windowSpaceMouseDelta = Globals::Components().mouseState().getMouseDelta();
		const glm::vec2 mouseDelta = { windowSpaceMouseDelta.x, -windowSpaceMouseDelta.y };

		turn(mouseDelta);
		throttle(Globals::Components().mouseState().rmb);
		magneticHook(Globals::Components().mouseState().mmb || Globals::Components().mouseState().xmb1);

		updatePlayersPositionsBuffers();
		updateConnectionsGraphicsBuffers();
	}

	void Players::render() const
	{
		basicRender();
		coloredRender();
		sceneCoordTexturedRender();
		customShadersRender();
	}

	void Players::basicRender() const
	{
		glUseProgram_proxy(Globals::Shaders().basic().getProgramId());
		Globals::Shaders().basic().vpUniform.setValue(Globals::Components().mvp().getVP());
		Globals::Shaders().basic().colorUniform.setValue(Globals::Components().graphicsSettings().defaultColor);
		Globals::Shaders().basic().modelUniform.setValue(glm::mat4(1.0f));

		glBindVertexArray(simplePlayersBuffers->positionBuffer);
		glDrawArrays(GL_TRIANGLES, 0, simplePlayersBuffers->positionsCache.size());

		for (const auto& customSimplePlayerBuffers : customSimplePlayersBuffers)
		{
			Globals::Shaders().basic().colorUniform.setValue(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().basic().modelUniform.setValue(glm::mat4(1.0f));

			std::function<void()> renderingTeardown =
				(*customSimplePlayerBuffers.renderingSetup)(Globals::Shaders().basic().getProgramId());

			glBindVertexArray(customSimplePlayerBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, customSimplePlayerBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}
	}

	void Players::sceneCoordTexturedRender() const
	{
		glUseProgram_proxy(Globals::Shaders().textured().getProgramId());
		Globals::Shaders().textured().vpUniform.setValue(Globals::Components().mvp().getVP());

		for (const auto& [texture, texturedPlayerBuffers] : texturesToPlayersBuffers)
		{
			Globals::Shaders().textured().colorUniform.setValue(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().textured().modelUniform.setValue(glm::mat4(1.0f));
			Tools::TexturedRender(Globals::Shaders().textured(), texturedPlayerBuffers, texture);
		}

		for (const auto& customTexturedPlayerBuffers : customTexturedPlayersBuffers)
		{
			Globals::Shaders().textured().colorUniform.setValue(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().textured().modelUniform.setValue(glm::mat4(1.0f));
			Tools::TexturedRender(Globals::Shaders().textured(), customTexturedPlayerBuffers,
				*customTexturedPlayerBuffers.texture);
		}
	}

	void Players::customShadersRender() const
	{
		for (const auto& currentBuffers : customShadersPlayersBuffers)
		{
			glUseProgram_proxy(*currentBuffers.customShadersProgram);

			std::function<void()> renderingTeardown;
			if (currentBuffers.renderingSetup)
				renderingTeardown = (*currentBuffers.renderingSetup)(*currentBuffers.customShadersProgram);

			glBindVertexArray(currentBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, currentBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}
	}

	void Players::coloredRender() const
	{
		glUseProgram_proxy(Globals::Shaders().colored().getProgramId());
		Globals::Shaders().colored().vpUniform.setValue(Globals::Components().mvp().getVP());
		Globals::Shaders().colored().colorUniform.setValue(Globals::Components().graphicsSettings().defaultColor);
		Globals::Shaders().colored().modelUniform.setValue(glm::mat4(1.0f));
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

			player.body->SetTransform(player.body->GetPosition(), playerAngle + controllerDot * mouseSensitivity);
		}

		if (player.grappleJoint)
		{
			const auto& grapple = Globals::Components().grapples()[player.connectedGrappleId];
			const glm::vec2 stepVelocity = (player.getCenter() - player.previousCenter) - (grapple.getCenter() - grapple.previousCenter);
			const float stepVelocityLength = glm::length(stepVelocity);

			if (stepVelocityLength > 0.0f)
			{
				const glm::vec2 normalizedStepVelocity = stepVelocity / stepVelocityLength;
				const float playerAngle = player.body->GetAngle();
				const float playerSideAngle = playerAngle + glm::half_pi<float>();
				const glm::vec2 playerDirection = { std::cos(playerSideAngle), std::sin(playerSideAngle) };
				const float velocityDot = glm::dot(playerDirection, normalizedStepVelocity);

				player.body->SetTransform(player.body->GetPosition(), playerAngle + velocityDot *
					stepVelocityLength * player.autoRotationFactor);
			}
		}
	}

	void Players::throttle(bool active) const
	{
		player.throttling = active;

		if (!active) return;

		const float currentAngle = player.body->GetAngle();
		player.body->ApplyForce(b2Vec2(glm::cos(currentAngle),
			glm::sin(currentAngle)) * playerForwardForce, player.body->GetWorldCenter(), true);
	}

	void Players::magneticHook(bool active)
	{
		connections.clear();

		int nearestGrappleId = -1;
		float nearestGrappleDistance = std::numeric_limits<float>::infinity();
		std::vector<int> grapplesInRange;

		for (int i = 0; i < (int)Globals::Components().grapples().size(); ++i)
		{
			const auto& grapple = Globals::Components().grapples()[i];
			const float grappleDistance = glm::distance(player.getCenter(), grapple.getCenter());

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

		for (const int grappleInRange : grapplesInRange)
		{
			const auto& grapple = Globals::Components().grapples()[grappleInRange];

			if (grappleInRange == nearestGrappleId)
			{
				if (active && !player.grappleJoint &&
					(glm::distance(player.getCenter(), grapple.getCenter()) >=
					glm::distance(player.previousCenter, grapple.previousCenter) || 
					player.connectIfApproaching))
				{
					player.connectedGrappleId = grappleInRange;
					player.weakConnectedGrappleId = -1;
					createGrappleJoint();
				}
				else if(player.connectedGrappleId != grappleInRange)
				{
					if (active)
					{
						if (player.grappleJoint)
						{
							connections.emplace_back(player.getCenter(), grapple.getCenter(),
								glm::vec4(0.0f, 1.0f, 0.0f, 0.2f), 1);
						}
						player.weakConnectedGrappleId = grappleInRange;
					}
					else
					{
						connections.emplace_back(player.getCenter(), grapple.getCenter(),
							glm::vec4(0.0f, 1.0f, 0.0f, 0.2f), 1);
					}
				}
			}
			else if (player.connectedGrappleId != grappleInRange)
			{
				connections.emplace_back(player.getCenter(), grapple.getCenter(), glm::vec4(1.0f, 0.0f, 0.0f, 0.2f), 1);
			}
		}

		if (player.connectedGrappleId != -1)
		{
			connections.emplace_back(player.getCenter(), Globals::Components().grapples()[player.connectedGrappleId].getCenter(),
				glm::vec4(0.0f, 0.0f, 1.0f, 0.7f), 20, 0.4f);
		}
		else if (player.weakConnectedGrappleId != -1)
		{
			connections.emplace_back(player.getCenter(), Globals::Components().grapples()[player.weakConnectedGrappleId].getCenter(),
				glm::vec4(0.0f, 0.0f, 1.0f, 0.5f), 1);
		}
	}

	void Players::createGrappleJoint() const
	{
		assert(player.connectedGrappleId != -1);

		const auto& grapple = Globals::Components().grapples()[player.connectedGrappleId];

		b2DistanceJointDef distanceJointDef;
		distanceJointDef.bodyA = player.body.get();
		distanceJointDef.bodyB = grapple.body.get();
		distanceJointDef.localAnchorA = distanceJointDef.bodyA->GetLocalCenter();
		distanceJointDef.localAnchorB = distanceJointDef.bodyB->GetLocalCenter();
		distanceJointDef.length = glm::distance(player.getCenter(), grapple.getCenter());
		distanceJointDef.collideConnected = true;
		player.grappleJoint.reset(Globals::Components().physics().world->CreateJoint(&distanceJointDef));
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

	std::vector<glm::vec3> Players::Connection::getPositions() const
	{
		if (segmentsNum == 1) return { { p1, 0.0f }, { p2, 0.0f } };
		else return Tools::CreatePositionsOfLightning(p1, p2, segmentsNum, frayFactor);
	}

	std::vector<glm::vec4> Players::Connection::getColors() const
	{
		return std::vector<glm::vec4>(segmentsNum * 2, color);
	}
}
