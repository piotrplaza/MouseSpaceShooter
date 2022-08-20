#include "b2Helpers.hpp"

#include <components/_typeComponentMappers.hpp>
#include <components/physics.hpp>

#include <globals/components.hpp>
#include <globals/collisionBits.hpp>

#include <tools/graphicsHelpers.hpp>

#include <commonTypes/bodyUserData.hpp>

#include <glm/gtx/transform.hpp>

namespace
{
	constexpr float playerLinearDamping = 0.1f;
	constexpr float playerAngularDamping = 15.0f;
	constexpr int circleGraphicsComplexity = 60;
}

void b2BodyDeleter::operator()(b2Body* body) const
{
	auto* bodyUserData = reinterpret_cast<BodyUserData*>(body->GetUserData().pointer);
	delete bodyUserData;
	body->GetWorld()->DestroyBody(body);
}

void b2JointDeleter::operator()(b2Joint* joint) const
{
	joint->GetBodyA()->GetWorld()->DestroyJoint(joint);
}

namespace Tools
{
	Body CreatePlaneBody(float size, float density, float spreadFactor)
	{
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(0.0f, 0.0f);
		bodyDef.angle = 0.0f;
		bodyDef.bullet = true;
		Body body(Globals::Components().physics().world->CreateBody(&bodyDef));

		b2FixtureDef fixtureDef;
		const b2Vec2 playerTriangle[3] = {
			{ size, 0 },
			{ -size * spreadFactor, size * spreadFactor },
			{ -size * spreadFactor, -size * spreadFactor }
		};
		b2PolygonShape polygonShape;
		polygonShape.Set(playerTriangle, 3);
		fixtureDef.shape = &polygonShape;
		fixtureDef.density = density;
		fixtureDef.restitution = 0.1f;
		body->CreateFixture(&fixtureDef);

		body->SetSleepingAllowed(false);
		body->SetLinearDamping(playerLinearDamping);
		body->SetAngularDamping(playerAngularDamping);

		body->GetUserData().pointer = reinterpret_cast<uintptr_t>(new BodyUserData);

		return body;
	}

	Body CreateBoxBody(glm::vec2 position, glm::vec2 hSize, float angle,
		b2BodyType bodyType, float density, float restitution, float friction)
	{
		b2BodyDef bodyDef;
		bodyDef.type = bodyType;
		bodyDef.position.Set(position.x, position.y);
		bodyDef.angle = angle;
		Body body(Globals::Components().physics().world->CreateBody(&bodyDef));

		b2FixtureDef fixtureDef;
		b2PolygonShape polygonShape;
		polygonShape.SetAsBox(hSize.x, hSize.y);
		fixtureDef.shape = &polygonShape;
		fixtureDef.density = density;
		fixtureDef.restitution = restitution;
		fixtureDef.friction = friction;
		fixtureDef.filter.categoryBits = Globals::CollisionBits::wallBit;
		body->CreateFixture(&fixtureDef);

		body->GetUserData().pointer = reinterpret_cast<uintptr_t>(new BodyUserData);

		return body;
	}

	Body CreateCircleBody(glm::vec2 position, float radius,
		b2BodyType bodyType, float density, float restitution, float friction)
	{
		b2BodyDef bodyDef;
		bodyDef.type = bodyType;
		bodyDef.position.Set(position.x, position.y);
		Body body(Globals::Components().physics().world->CreateBody(&bodyDef));

		b2FixtureDef fixtureDef;
		b2CircleShape circleShape;
		circleShape.m_radius = radius;
		fixtureDef.shape = &circleShape;
		fixtureDef.density = density;
		fixtureDef.restitution = restitution;
		fixtureDef.friction = friction;
		fixtureDef.filter.categoryBits = Globals::CollisionBits::wallBit;
		body->CreateFixture(&fixtureDef);

		body->GetUserData().pointer = reinterpret_cast<uintptr_t>(new BodyUserData);

		return body;
	}

	void CreateRevoluteJoint(b2Body& body1, b2Body& body2, glm::vec2 pinPoint, bool collideConnected)
	{
		b2RevoluteJointDef revoluteJointDef;
		revoluteJointDef.bodyA = &body1;
		revoluteJointDef.bodyB = &body2;
		revoluteJointDef.localAnchorA = body1.GetLocalPoint({ pinPoint.x, pinPoint.y });
		revoluteJointDef.localAnchorB = body2.GetLocalPoint({ pinPoint.x, pinPoint.y });
		revoluteJointDef.collideConnected = collideConnected;
		Globals::Components().physics().world->CreateJoint(&revoluteJointDef);
	}

	void CreateDistanceJoint(b2Body& body1, b2Body& body2, glm::vec2 pinPoint, bool collideConnected, float length)
	{
		b2DistanceJointDef distanceJointDef;
		distanceJointDef.bodyA = &body1;
		distanceJointDef.bodyB = &body2;
		distanceJointDef.localAnchorA = body1.GetLocalPoint({ pinPoint.x, pinPoint.y });
		distanceJointDef.localAnchorB = body2.GetLocalPoint({ pinPoint.x, pinPoint.y });
		distanceJointDef.collideConnected = collideConnected;
		distanceJointDef.length = length;
		distanceJointDef.minLength = length;
		distanceJointDef.maxLength = length;
		Globals::Components().physics().world->CreateJoint(&distanceJointDef);
	}

	glm::mat4 GetModelMatrix(const b2Body& body)
	{
		const auto& bodyTransform = body.GetTransform();
		return glm::rotate(glm::translate(glm::mat4(1.0f), { bodyTransform.p.x, bodyTransform.p.y, 0.0f }),
			bodyTransform.q.GetAngle(), { 0.0f, 0.0f, 1.0f });
	}

	std::vector<glm::vec3> GetVertices(const b2Body& body)
	{
		std::vector<glm::vec3> vertices;

		auto* fixture = body.GetFixtureList();
		do
		{
			switch (fixture->GetType())
			{
			case b2Shape::e_polygon:
			{
				const auto& polygonShape = static_cast<const b2PolygonShape&>(*fixture->GetShape());
				switch (polygonShape.m_count)
				{
				case 3:
				{
					vertices.reserve(vertices.size() + 3);
					for (int i = 0; i < 3; ++i)
					{
						const auto& b2v = polygonShape.m_vertices[i];
						vertices.emplace_back(b2v.x, b2v.y, 0.0f);
					}
					break;
				}
				case 4:
				{
					vertices.reserve(vertices.size() + 6);
					for (int i = 0; i < 6; ++i)
					{
						const auto& b2v = polygonShape.m_vertices[i < 3 ? i : (i - 1) % 4];
						vertices.emplace_back(b2v.x, b2v.y, 0.0f);
					}
					break;
				}
				default:
					assert(!"unsupported vertices count");
				}
				break;
			}
			case b2Shape::e_circle:
			{
				const auto& circleShape = static_cast<const b2CircleShape&>(*fixture->GetShape());
				Tools::AppendVerticesOfCircle(vertices, ToVec2<glm::vec2>(circleShape.m_p), circleShape.m_radius, circleGraphicsComplexity);
				break;
			}
			default:
				assert(!"unsupported shape");
			}
		}
		while (fixture = fixture->GetNext());

		return vertices;
	}

	void SetCollisionFilteringBits(b2Body& body, unsigned short categoryBits, unsigned short maskBits)
	{
		auto* fixture = body.GetFixtureList();
		while (fixture)
		{
			auto filterData = fixture->GetFilterData();
			filterData.categoryBits = categoryBits;
			filterData.maskBits = maskBits;
			fixture->SetFilterData(filterData);

			fixture = fixture->GetNext();
		}
	}

	std::pair<const b2Fixture*, const b2Fixture*> Sort(const b2Fixture* fixtureA, const b2Fixture* fixtureB)
	{
		return fixtureA > fixtureB ? std::make_pair(fixtureB, fixtureA) : std::make_pair(fixtureA, fixtureB);
	}

	std::pair<const unsigned short, const unsigned short> Sort(const unsigned short collisionObjectA, const unsigned short collisionObjectB)
	{
		return collisionObjectA > collisionObjectB ? std::make_pair(collisionObjectB, collisionObjectA) : std::make_pair(collisionObjectA, collisionObjectB);
	}

	BodyUserData& AccessUserData(b2Body& body)
	{
		return *reinterpret_cast<BodyUserData*>(body.GetUserData().pointer);
	}

	const BodyUserData& AccessUserData(const b2Body& body)
	{
		return *reinterpret_cast<BodyUserData*>(const_cast<b2Body&>(body).GetUserData().pointer);
	}
}
