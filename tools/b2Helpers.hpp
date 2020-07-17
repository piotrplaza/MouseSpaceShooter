#pragma once

#include <memory>

#include <Box2D/Box2D.h>

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

struct b2BodyDeleter {
	void operator()(b2Body* body) const;
};

struct b2JointDeleter {
	void operator()(b2Joint* joint) const;
};

template <typename TargetVec2, typename SourceVec2>
inline TargetVec2 ToVec2(const SourceVec2 v)
{
	return { v.x, v.y };
}

inline b2Vec2 operator *(const b2Vec2 v, const float s)
{
	return { v.x * s, v.y * s };
}

namespace Tools
{
	std::unique_ptr<b2Body, b2BodyDeleter> CreateTrianglePlayerBody(float size, float density, float spreadFactor = 0.5f);
	std::unique_ptr<b2Body, b2BodyDeleter> CreateBoxBody(glm::vec2 position, glm::vec2 hSize, float angle = 0.0f,
		b2BodyType bodyType = b2_staticBody, float density = 1.0f, float restitution = 0.1f, float friction = 0.2f);
	std::unique_ptr<b2Body, b2BodyDeleter> CreateCircleBody(glm::vec2 position, float radius,
		b2BodyType bodyType = b2_staticBody, float density = 1.0f, float restitution = 0.1f, float friction = 0.2f);
	void PinBodies(b2Body& body1, b2Body& body2, glm::vec2 pinPoint, bool collideConnected = false);
	glm::mat4 GetModelMatrix(const b2Body& body);
}
