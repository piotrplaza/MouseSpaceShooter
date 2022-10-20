#include "b2Helpers.hpp"

#include <components/physics.hpp>

#include <globals/components.hpp>

#include <tools/graphicsHelpers.hpp>

#include <commonTypes/typeComponentMappers.hpp>
#include <commonTypes/bodyUserData.hpp>

#include <glm/gtx/transform.hpp>

namespace
{
	constexpr float playerLinearDamping = 0.1f;
	constexpr float playerAngularDamping = 15.0f;
	constexpr int circleGraphicsComplexity = 60;

	template<size_t Size>
	inline void CreatePolygonShapedFixture(Body& body, const std::array<glm::vec2, Size>& vertices, float density, float restitution, float friction,
		unsigned short collisionBits, bool sensor)
	{
		b2FixtureDef fixtureDef;
		b2PolygonShape polygonShape;

		// UB: Breaks Strict Aliasing Rule, but is it better way to avoid copying?
		polygonShape.Set(reinterpret_cast<const b2Vec2*>(&vertices[0]), vertices.size());
		fixtureDef.shape = &polygonShape;
		fixtureDef.density = density;
		fixtureDef.restitution = restitution;
		fixtureDef.friction = friction;
		fixtureDef.filter.categoryBits = collisionBits;
		fixtureDef.isSensor = sensor;

		body->CreateFixture(&fixtureDef);
	}
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
	Body CreateEmptyBody(const BodyParams& bodyParams)
	{
		b2BodyDef bodyDef;
		bodyDef.type = bodyParams.bodyType_;
		bodyDef.position.Set(bodyParams.position_.x, bodyParams.position_.y);
		bodyDef.angle = bodyParams.angle_;
		Body body(Globals::Components().physics().world->CreateBody(&bodyDef));

		body->GetUserData().pointer = reinterpret_cast<uintptr_t>(new BodyUserData);

		return body;
	}

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

	Body CreateBoxBody(glm::vec2 hSize, const BodyParams& bodyParams)
	{
		Body body = CreateEmptyBody(bodyParams);

		b2FixtureDef fixtureDef;
		b2PolygonShape polygonShape;
		polygonShape.SetAsBox(hSize.x, hSize.y);
		fixtureDef.shape = &polygonShape;
		fixtureDef.density = bodyParams.density_;
		fixtureDef.restitution = bodyParams.restitution_;
		fixtureDef.friction = bodyParams.friction_;
		fixtureDef.filter.categoryBits = bodyParams.categoryBits_;
		fixtureDef.isSensor = bodyParams.sensor_;
		body->CreateFixture(&fixtureDef);

		return body;
	}

	Body CreateCircleBody(float radius, const BodyParams& bodyParams)
	{

		Body body = CreateEmptyBody(bodyParams);

		b2FixtureDef fixtureDef;
		b2CircleShape circleShape;
		circleShape.m_radius = radius;
		fixtureDef.shape = &circleShape;
		fixtureDef.density = bodyParams.density_;
		fixtureDef.restitution = bodyParams.restitution_;
		fixtureDef.friction = bodyParams.friction_;
		fixtureDef.filter.categoryBits = bodyParams.categoryBits_;
		fixtureDef.isSensor = bodyParams.sensor_;
		body->CreateFixture(&fixtureDef);

		return body;
	}

	Body CreateConvex4Body(const std::array<glm::vec2, 4>& vertices, const BodyParams& bodyParams)
	{
		Body body = CreateEmptyBody(bodyParams);

		CreatePolygonShapedFixture(body, vertices, bodyParams.density_, bodyParams.restitution_, bodyParams.friction_, bodyParams.categoryBits_, bodyParams.sensor_);

		return body;
	}

	Body CreateTrianglesBody(const std::vector<std::array<glm::vec2, 3>>& vertices, const BodyParams& bodyParams)
	{
		Body body = CreateEmptyBody(bodyParams);

		for (const auto& triangle : vertices)
			CreatePolygonShapedFixture(body, triangle, bodyParams.density_, bodyParams.restitution_, bodyParams.friction_, bodyParams.categoryBits_, bodyParams.sensor_);

		return body;
	}

	Body CreatePolylineBody(const std::vector<glm::vec2>& vertices, const BodyParams& bodyParams)
	{
		assert(vertices.size() >= 2);

		Body body = CreateEmptyBody(bodyParams);
		CreatePolylineFixtures(body, vertices, bodyParams);

		return body;
	}

	void CreatePolylineFixtures(Body& body, const std::vector<glm::vec2>& vertices, const BodyParams& bodyParams)
	{
		for (auto it = vertices.begin(); it != std::prev(vertices.end()); ++it)
		{
			auto& v1 = *it;
			auto& v2 = *(it + 1);

			b2FixtureDef fixtureDef;
			b2EdgeShape edgeShape;

			edgeShape.SetTwoSided(ToVec2<b2Vec2>(v1), ToVec2<b2Vec2>(v2));
			fixtureDef.shape = &edgeShape;
			fixtureDef.density = bodyParams.density_;
			fixtureDef.restitution = bodyParams.restitution_;
			fixtureDef.friction = bodyParams.friction_;
			fixtureDef.filter.categoryBits = bodyParams.categoryBits_;
			fixtureDef.isSensor = bodyParams.sensor_;

			body->CreateFixture(&fixtureDef);
		}
	}

	b2Joint* CreateRevoluteJoint(b2Body& body1, b2Body& body2, glm::vec2 pinPoint, bool collideConnected)
	{
		b2RevoluteJointDef revoluteJointDef;
		revoluteJointDef.bodyA = &body1;
		revoluteJointDef.bodyB = &body2;
		revoluteJointDef.localAnchorA = body1.GetLocalPoint({ pinPoint.x, pinPoint.y });
		revoluteJointDef.localAnchorB = body2.GetLocalPoint({ pinPoint.x, pinPoint.y });
		revoluteJointDef.collideConnected = collideConnected;
		return Globals::Components().physics().world->CreateJoint(&revoluteJointDef);
	}

	b2Joint* CreateDistanceJoint(b2Body& body1, b2Body& body2, glm::vec2 body1Anchor, glm::vec2 body2Anchor, bool collideConnected, float length)
	{
		b2DistanceJointDef distanceJointDef;
		distanceJointDef.bodyA = &body1;
		distanceJointDef.bodyB = &body2;
		distanceJointDef.localAnchorA = body1.GetLocalPoint({ body1Anchor.x, body1Anchor.y });
		distanceJointDef.localAnchorB = body2.GetLocalPoint({ body2Anchor.x, body2Anchor.y });
		distanceJointDef.collideConnected = collideConnected;
		distanceJointDef.length = length;
		distanceJointDef.minLength = length;
		distanceJointDef.maxLength = length;
		return Globals::Components().physics().world->CreateJoint(&distanceJointDef);
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
		while(fixture)
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
			case b2Shape::e_edge:
			{
				const auto& edgeShape = static_cast<const b2EdgeShape&>(*fixture->GetShape());
				vertices.emplace_back(edgeShape.m_vertex1.x, edgeShape.m_vertex1.y, 0.0f);
				vertices.emplace_back(edgeShape.m_vertex2.x, edgeShape.m_vertex2.y, 0.0f);
				break;
			}
			default:
				assert(!"unsupported shape");
			}

			fixture = fixture->GetNext();
		}

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

	std::optional<glm::vec2> GetCollisionPoint(const b2Body& body1, const b2Body& body2)
	{
		for (auto contactEdge = body1.GetContactList(); contactEdge != nullptr; contactEdge = contactEdge->next)
		{
			if (contactEdge->contact->IsTouching())
			{
				if ((contactEdge->contact->GetFixtureA()->GetBody() == &body1 &&
					contactEdge->contact->GetFixtureB()->GetBody() == &body2)
					||
					(contactEdge->contact->GetFixtureA()->GetBody() == &body2 &&
						contactEdge->contact->GetFixtureB()->GetBody() == &body1))
				{
					b2WorldManifold worldManifold;
					contactEdge->contact->GetWorldManifold(&worldManifold);

					return ToVec2<glm::vec2>(worldManifold.points[0]);
				}
			}
		}
		return std::nullopt;
	}

	float GetRelativeVelocity(const b2Body& body1, const b2Body& body2)
	{
		return glm::length(ToVec2<glm::vec2>(body2.GetLinearVelocity()) - ToVec2<glm::vec2>(body1.GetLinearVelocity()));
	}

	void DestroyFixtures(Body& body)
	{
		auto* fixture = body->GetFixtureList();
		while (fixture)
		{
			auto* currentFixture = fixture;
			fixture = fixture->GetNext();
			body->DestroyFixture(currentFixture);
		}
	}
}
