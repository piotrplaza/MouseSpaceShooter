#include "player.hpp"

#include <vector>
#include <limits>
#include <type_traits>

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
		initPhysics();
		initGraphics();
	}

	void Player::initPhysics() const
	{
		using namespace Globals::Components;
		using namespace Globals::Defaults;

		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(0.0f, 0.0f);
		bodyDef.angle = 0.0f;
		player.body.reset(physics.world.CreateBody(&bodyDef));

		b2FixtureDef fixtureDef;
		const float playerSize = 1.0f;
		const b2Vec2 playerTriangle[3] = {
			{ playerSize, 0 },
			{ -playerSize / 2.0f, playerSize / 2.0f },
			{ -playerSize / 2.0f, -playerSize / 2.0f }
		};
		b2PolygonShape polygonShape;
		polygonShape.Set(playerTriangle, 3);
		fixtureDef.shape = &polygonShape;
		fixtureDef.density = 1.0f;
		fixtureDef.restitution = 0.1f;
		player.body->CreateFixture(&fixtureDef);

		player.body->SetSleepingAllowed(false);

		player.body->SetLinearDamping(playerLinearDamping);
		player.body->SetAngularDamping(playerAngularDamping);
	}

	void Player::initGraphics()
	{
		using namespace Globals::Components;

		shadersProgram = shaders::LinkProgram(shaders::CompileShaders("shaders/basic.vs", "shaders/basic.fs"),
			{ {0, "bPos"} });

		glCreateVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		player.updateVerticesCache();
		glBufferData(GL_ARRAY_BUFFER, player.verticesCache.size() * sizeof(player.verticesCache.front()), player.verticesCache.data(), GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
	}

	void Player::step() const
	{
		using namespace Globals::Components;

		const glm::ivec2 windowSpaceMouseDelta = mouseState.getMouseDelta();
		const glm::vec2 mouseDelta = { windowSpaceMouseDelta.x, -windowSpaceMouseDelta.y };

		turn(mouseDelta);
		throttle(mouseState.rmb);
		magneticHook(mouseState.lmb);
	}

	void Player::render() const
	{
		glUseProgram(shadersProgram);
		glUniformMatrix4fv(glGetUniformLocation(shadersProgram, "mvp"), 1, GL_FALSE,
			glm::value_ptr(Globals::Components::mvp.getMVP(Globals::Components::player.getModelMatrix())));

		glBindVertexArray(vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, Globals::Components::player.verticesCache.size());
	}

	void Player::turn(glm::vec2 controllerDelta) const
	{
		using namespace Globals::Components;

		if (glm::length(controllerDelta) > 0)
		{
			const float playerAngle = player.body->GetAngle();
			const float playerSideAngle = playerAngle + glm::half_pi<float>();
			const glm::vec2 playerDirection = { std::cos(playerSideAngle), std::sin(playerSideAngle) };
			const float dot = glm::dot(playerDirection, controllerDelta);

			player.body->SetTransform(player.body->GetPosition(), playerAngle + dot * Globals::Defaults::mouseSensitivity);
		}
	}

	void Player::throttle(bool active) const
	{
		using namespace Globals::Components;
		using namespace Globals::Defaults;

		if (!active) return;

		const float currentAngle = player.body->GetAngle();
		player.body->ApplyForce(b2Vec2(glm::cos(currentAngle),
			glm::sin(currentAngle)) * playerForwardForce, player.body->GetWorldCenter(), true);
	}

	void Player::magneticHook(bool active) const
	{
		using namespace Globals::Components;

		connections.clear();

		auto nearestGrappleIt = grapples.end();
		float nearestDistance = std::numeric_limits<float>::infinity();
		std::vector<std::remove_reference<decltype(grapples)>::type::iterator> grapplesInRange;

		for (auto it = grapples.begin(); it != grapples.end(); ++it)
		{
			const float distance = glm::distance(player.getPosition(), it->getPosition());

			if (distance > it->influenceRadius) continue;

			grapplesInRange.push_back(it);

			if (distance < nearestDistance)
			{
				nearestDistance = distance;
				nearestGrappleIt = it;
			}
		}

		for (auto it = grapplesInRange.begin(); it != grapplesInRange.end(); ++it)
		{
			if (*it == nearestGrappleIt)
			{
				if (active)
				{
					connections.emplace_back(player.getPosition(), (*it)->getPosition(), glm::vec4(0.0f, 0.0f, 1.0f, 0.7f), 20, 0.5f);
				}
				else
				{
					connections.emplace_back(player.getPosition(), (*it)->getPosition(), glm::vec4(1.0f, 1.0f, 1.0f, 0.5f), 1);
				}
			}
			else
			{
				connections.emplace_back(player.getPosition(), (*it)->getPosition(), glm::vec4(1.0f, 1.0f, 1.0f, 0.2f), 1);
			}
		}
	}
}
