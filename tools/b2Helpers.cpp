#include "b2Helpers.hpp"

#include "globals.hpp"

#include "components/physics.hpp"

void b2BodyDeleter::operator()(b2Body * body) const
{
		body->GetWorld()->DestroyBody(body);
}

namespace tools
{
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
