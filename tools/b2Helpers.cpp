#include "b2Helpers.hpp"

#include <glm/gtx/transform.hpp>

#include <globals.hpp>

#include <components/physics.hpp>

#include <tools/graphicsHelpers.hpp>

namespace
{
	constexpr float playerLinearDamping = 0.1f;
	constexpr float playerAngularDamping = 15.0f;
	constexpr int circleGraphicsComplexity = 60;
}

void b2BodyDeleter::operator()(b2Body * body) const
{
		body->GetWorld()->DestroyBody(body);
}

void b2JointDeleter::operator()(b2Joint* joint) const
{
	joint->GetBodyA()->GetWorld()->DestroyJoint(joint);
}

namespace Tools
{
	std::unique_ptr<b2Body, b2BodyDeleter> CreateTrianglePlayerBody(float size, float density, float spreadFactor)
	{
		using namespace Globals::Components;

		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(0.0f, 0.0f);
		bodyDef.angle = 0.0f;
		bodyDef.bullet = true;
		std::unique_ptr<b2Body, b2BodyDeleter> playerBody(physics.world.CreateBody(&bodyDef));

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
		playerBody->CreateFixture(&fixtureDef);

		playerBody->SetSleepingAllowed(false);
		playerBody->SetLinearDamping(playerLinearDamping);
		playerBody->SetAngularDamping(playerAngularDamping);

		return playerBody;
	}

	std::unique_ptr<b2Body, b2BodyDeleter> CreateBoxBody(glm::vec2 position, glm::vec2 hSize, float angle,
		b2BodyType bodyType, float density, float restitution, float friction)
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
		fixtureDef.restitution = restitution;
		fixtureDef.friction = friction;
		body->CreateFixture(&fixtureDef);

		return body;
	}

	std::unique_ptr<b2Body, b2BodyDeleter> CreateCircleBody(glm::vec2 position, float radius,
		b2BodyType bodyType, float density, float restitution, float friction)
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
		fixtureDef.restitution = restitution;
		fixtureDef.friction = friction;
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

	glm::mat4 GetModelMatrix(const b2Body& body)
	{
		const auto& bodyTransform = body.GetTransform();
		return glm::rotate(glm::translate(glm::mat4(1.0f), { bodyTransform.p.x, bodyTransform.p.y, 0.0f }),
			bodyTransform.q.GetAngle(), { 0.0f, 0.0f, 1.0f });
	}

	std::vector<glm::vec3> GetPositions(const b2Body& body)
	{
		std::vector<glm::vec3> positions;

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
					positions.reserve(positions.size() + 3);
					for (int i = 0; i < 3; ++i)
					{
						const auto& b2v = polygonShape.m_vertices[i];
						positions.emplace_back(b2v.x, b2v.y, 0.0f);
					}
					break;
				}
				case 4:
				{
					positions.reserve(positions.size() + 6);
					for (int i = 0; i < 6; ++i)
					{
						const auto& b2v = polygonShape.m_vertices[i < 3 ? i : (i - 1) % 4];
						positions.emplace_back(b2v.x, b2v.y, 0.0f);
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
				Tools::AppendPositionsOfCircle(positions, ToVec2<glm::vec2>(circleShape.m_p), circleShape.m_radius, circleGraphicsComplexity);
				break;
			}
			default:
				assert(!"unsupported shape");
			}
		}
		while (fixture = fixture->GetNext());

		return positions;
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
}
