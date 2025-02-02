#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <vector>

inline glm::vec2 operator +(const glm::vec2& lhs, const glm::ivec2& rhs)
{
	return { lhs.x + rhs.x, lhs.y + rhs.y };
}

inline glm::vec2 operator -(const glm::vec2& lhs, const glm::ivec2& rhs)
{
	return { lhs.x - rhs.x, lhs.y - rhs.y };
}

inline glm::vec2 operator *(const glm::ivec2& lhs, const glm::vec2& rhs)
{
	return { lhs.x * rhs.x, lhs.y * rhs.y };
}

inline glm::vec2 operator /(const glm::vec2& lhs, const glm::ivec2& rhs)
{
	return { lhs.x / rhs.x, lhs.y / rhs.y };
}

inline glm::vec2 operator +(const glm::ivec2& lhs, float rhs)
{
	return { lhs.x + rhs, lhs.y + rhs };
}

inline glm::vec2 operator -(const glm::ivec2& lhs, float rhs)
{
	return { lhs.x - rhs, lhs.y - rhs };
}

inline glm::vec2 operator *(const glm::ivec2& lhs, float rhs)
{
	return { lhs.x * rhs, lhs.y * rhs };
}

inline glm::vec2 operator /(const glm::ivec2& lhs, float rhs)
{
	return { lhs.x / rhs, lhs.y / rhs };
}


inline glm::vec2 operator /(const glm::ivec2& lhs, const glm::vec2& rhs)
{
	return { lhs.x / rhs.x, lhs.y / rhs.y };
}

inline glm::vec3 operator /(const glm::ivec3& lhs, const glm::vec3& rhs)
{
	return { lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z };
}

inline glm::vec4 operator /(const glm::ivec4& lhs, const glm::vec4& rhs)
{
	return { lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w };
}

namespace Tools
{
	inline std::vector<glm::vec3> ConvertToVec3Vector(const std::vector<glm::vec2>& sourceVec)
	{
		std::vector<glm::vec3> result;
		result.reserve(sourceVec.size());
		for (const auto e : sourceVec)
			result.emplace_back(e, 0.0f);
		return result;
	}

	template <typename V2>
	inline V2 Rotate90CW(const V2& vec)
	{
		return { vec.y, -vec.x };
	}

	template <typename V2>
	inline V2 Rotate90CCW(const V2& vec)
	{
		return { -vec.y, vec.x };
	}

	template <typename V2>
	inline V2 Rotate180(const V2& vec)
	{
		return -vec;
	}

	template <typename V2>
	inline V2 Rotate(const V2& vec, float angle)
	{
		const float cosA = glm::cos(angle);
		const float sinA = glm::sin(angle);
		return { vec.x * cosA - vec.y * sinA, vec.x * sinA + vec.y * cosA };
	}

	inline bool IsPointBetweenVectors(const glm::vec2& p, const glm::vec2& o,
		const glm::vec2& nv1, const glm::vec2& nv2)
	{
		glm::vec2 d = p - o;
		if (glm::length(d) < 1e-6f)
			return true;

		glm::vec2 nd = glm::normalize(d);

		float angleP = glm::orientedAngle(nv1, nd);
		float angleV2 = glm::orientedAngle(nv1, nv2);

		return (angleP * angleV2 >= 0) && (std::abs(angleP) <= std::abs(angleV2));
	}

	inline bool DoesRayIntersectCircle(const glm::vec2& o, const glm::vec2& v,
		const glm::vec2& c, float r)
	{
		const glm::vec2 oc = o - c;
		const float oc2 = glm::dot(oc, oc);
		if (oc2 <= r * r)
			return true;
		const float a = glm::dot(v, v);
		if (a < 1e-6f)
			return false;
		const float t = -glm::dot(oc, v) / a;
		if (t < 0.0f)
			return false;
		const float d2 = oc2 - (glm::dot(oc, v) * glm::dot(oc, v)) / a;
		return d2 <= r * r;
	}
}
