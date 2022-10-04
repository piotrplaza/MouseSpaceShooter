#pragma once

#include <globals/collisionBits.hpp>

#include <Box2D/Box2D.h>

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <memory>
#include <vector>
#include <array>
#include <utility>
#include <optional>

struct BodyUserData;

struct b2BodyDeleter {
	void operator()(b2Body* body) const;
};

struct b2JointDeleter {
	void operator()(b2Joint* joint) const;
};

template <typename TargetVec2>
inline TargetVec2 ToVec2(const auto v)
{
	return { v.x, v.y };
}

inline b2Vec2 operator *(const b2Vec2 v, const float s)
{
	return { v.x * s, v.y * s };
}

inline b2Vec2 operator /(const b2Vec2 v, const float s)
{
	return { v.x / s, v.y / s };
}

using Body = std::unique_ptr<b2Body, b2BodyDeleter>;

namespace Tools
{
	struct BodyParams
	{
		BodyParams& position(glm::vec2 value)
		{
			position_ = value;
			return *this;
		}

		BodyParams& angle(float value)
		{
			angle_ = value;
			return *this;
		}

		BodyParams& bodyType(b2BodyType value)
		{
			bodyType_ = value;
			return *this;
		}

		BodyParams& density(float value)
		{
			density_ = value;
			return *this;
		}

		BodyParams& restitution(float value)
		{
			restitution_ = value;
			return *this;
		}

		BodyParams& friction(float value)
		{
			friction_ = value;
			return *this;
		}

		BodyParams& categoryBits(uint16 value)
		{
			categoryBits_ = value;
			return *this;
		}

		BodyParams& sensor(bool value)
		{
			sensor_ = value;
			return *this;
		}

		glm::vec2 position_ = { 0.0f, 0.0f };
		float angle_ = 0.0f;
		b2BodyType bodyType_ = b2_staticBody;
		float density_ = 1.0f;
		float restitution_ = 0.1f;
		float friction_ = 0.2f;
		uint16 categoryBits_ = Globals::CollisionBits::wall;
		bool sensor_ = false;
	};

	Body CreatePlaneBody(float size, float density, float spreadFactor);
	Body CreateBoxBody(glm::vec2 hSize, BodyParams bodyParams);
	Body CreateCircleBody(float radius, BodyParams bodyParams);
	Body CreateConvex4Body(const std::array<glm::vec2, 4>& vertices, BodyParams bodyParams);
	Body CreateTrianglesBody(const std::vector<std::array<glm::vec2, 3>>& vertices, BodyParams bodyParams);
	Body CreatePolylineBody(const std::vector<glm::vec2>& vertices, BodyParams bodyParams = BodyParams{});
	b2Joint* CreateRevoluteJoint(b2Body& body1, b2Body& body2, glm::vec2 pinPoint, bool collideConnected = false);
	b2Joint* CreateDistanceJoint(b2Body& body1, b2Body& body2, glm::vec2 body1Anchor, glm::vec2 body2Anchor, bool collideConnected = false, float length = 0.0f);
	glm::mat4 GetModelMatrix(const b2Body& body);
	std::vector<glm::vec3> GetVertices(const b2Body& body);
	void SetCollisionFilteringBits(b2Body& body, unsigned short categoryBits, unsigned short maskBits);
	std::pair<const b2Fixture*, const b2Fixture*> Sort(const b2Fixture* fixtureA, const b2Fixture* fixtureB);
	std::pair<const unsigned short, const unsigned short> Sort(const unsigned short collisionObjectA, const unsigned short collisionObjectB);
	BodyUserData& AccessUserData(b2Body& body);
	const BodyUserData& AccessUserData(const b2Body& body);
	std::optional<glm::vec2> GetCollisionPoint(const b2Body& body1, const b2Body& body2);
	float GetRelativeVelocity(const b2Body& body1, const b2Body& body2);
}
