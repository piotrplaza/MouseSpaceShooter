#include "player.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <globals.hpp>

#include <components/player.hpp>
#include <components/physics.hpp>
#include <components/mvp.hpp>
#include <components/mouseState.hpp>

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

		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(0.0f, 0.0f);
		bodyDef.angle = 0.0f;

		const float playerSize = 1.0f;
		const b2Vec2 playerTriangle[3] = {
			{ playerSize, 0 },
			{ -playerSize / 2.0f, playerSize / 2.0f },
			{ -playerSize / 2.0f, -playerSize / 2.0f }
		};
		b2PolygonShape polygonShape;
		polygonShape.Set(playerTriangle, 3);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &polygonShape;
		fixtureDef.density = 1.0f;

		player.body.reset(physics.world.CreateBody(&bodyDef));
		player.body->CreateFixture(&fixtureDef);
		player.body->SetFixedRotation(true);
	}

	void Player::initGraphics()
	{
		using namespace Globals::Components;

		shadersProgram = shaders::LinkProgram(shaders::CompileShaders("shaders/basic.vs", "shaders/basic.fs"),
			{ {0, "bPos"} });

		glCreateVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		GLuint vertexBuffer;
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		player.updateVerticesCache();
		glBufferData(GL_ARRAY_BUFFER, player.verticesCache.size() * sizeof(player.verticesCache.front()), player.verticesCache.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void Player::step() const
	{
		using namespace Globals::Components;

		const glm::ivec2 windowSpaceMouseDelta = mouseState.getMouseDelta();
		const glm::vec2 mouseDelta = { windowSpaceMouseDelta.x, -windowSpaceMouseDelta.y };

		if (glm::length(mouseDelta) > 0)
		{
			const float playerAngle = player.body->GetAngle();
			const float playerSideAngle = playerAngle + glm::half_pi<float>();
			const glm::vec2 playerDirection = { std::cos(playerSideAngle), std::sin(playerSideAngle) };
			const float dot = glm::dot(playerDirection, mouseDelta);

			player.body->SetTransform(player.body->GetPosition(), playerAngle + dot * Globals::Defaults::mouseSensitivity);
		}

		if (mouseState.rmb)
		{
			const float force = 10.0f;
			const float currentAngle = player.body->GetAngle();

			player.body->ApplyForce(b2Vec2(glm::cos(currentAngle),
				glm::sin(currentAngle)) * force, player.body->GetWorldCenter(), true);
		}
	}

	void Player::render() const
	{
		glUseProgram(shadersProgram);
		glUniformMatrix4fv(glGetUniformLocation(shadersProgram, "mvp"), 1, GL_FALSE,
			glm::value_ptr(Globals::Components::mvp.getMVP(Globals::Components::player.getModelMatrix())));

		glBindVertexArray(vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, Globals::Components::player.verticesCache.size());
	}
}
