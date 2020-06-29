#include "b2Helpers.hpp"

#include "globals.hpp"

#include "components/physics.hpp"

void b2BodyDeleter::operator()(b2Body * body) const
{
		body->GetWorld()->DestroyBody(body);
}

void b2JointDeleter::operator()(b2Joint* joint) const
{
	joint->GetBodyA()->GetWorld()->DestroyJoint(joint);
}

namespace tools
{
	std::unique_ptr<b2Body, b2BodyDeleter> CreateBasicPlayerBody()
	{
		using namespace Globals::Components;
		using namespace Globals::Defaults;

		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(0.0f, 0.0f);
		bodyDef.angle = 0.0f;
		std::unique_ptr<b2Body, b2BodyDeleter> playerBody(physics.world.CreateBody(&bodyDef));

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
		playerBody->CreateFixture(&fixtureDef);

		playerBody->SetSleepingAllowed(false);
		playerBody->SetLinearDamping(playerLinearDamping);
		playerBody->SetAngularDamping(playerAngularDamping);

		return playerBody;
	}

	std::unique_ptr<b2Body, b2BodyDeleter> CreateBoxBody(glm::vec2 position, glm::vec2 hSize, float angle,
		b2BodyType bodyType, float density)
	{
		using namespace Globals::Components;

		b2BodyDef bodyDef;
		bodyDef.type = bodyType;
		bodyDef.position.Set(position.x, position.y);
		bodyDef.angle = angle;
		std::unique_ptr<b2Body, b2BodyDeleter> body(physics.world.CreateBody(&bodyDef));

		b2FixtureDef fixtureDef;
		b2PolygonShape polygonShape;
		polygonShape.SetAsBox(hSize.x, hSize.y);
		fixtureDef.shape = &polygonShape;
		fixtureDef.density = density;
		body->CreateFixture(&fixtureDef);

		return body;
	}

	std::unique_ptr<b2Body, b2BodyDeleter> CreateCircleBody(glm::vec2 position, float radius,
		b2BodyType bodyType, float density)
	{
		using namespace Globals::Components;

		b2BodyDef bodyDef;
		bodyDef.type = bodyType;
		bodyDef.position.Set(position.x, position.y);
		std::unique_ptr<b2Body, b2BodyDeleter> body(physics.world.CreateBody(&bodyDef));

		b2FixtureDef fixtureDef;
		b2CircleShape circleShape;
		circleShape.m_radius = radius;
		fixtureDef.shape = &circleShape;
		fixtureDef.density = density;
		body->CreateFixture(&fixtureDef);

		return body;
	}

	void PinBodies(b2Body& body1, b2Body& body2, glm::vec2 pinPoint, bool collideConnected)
	{
		using namespace Globals::Components;

		b2RevoluteJointDef revoluteJointDef;
		revoluteJointDef.bodyA = &body1;
		revoluteJointDef.bodyB = &body2;
		revoluteJointDef.localAnchorA = body1.GetLocalPoint({ pinPoint.x, pinPoint.y });
		revoluteJointDef.localAnchorB = body2.GetLocalPoint({ pinPoint.x, pinPoint.y });
		revoluteJointDef.collideConnected = collideConnected;
		physics.world.CreateJoint(&revoluteJointDef);
	}
}
