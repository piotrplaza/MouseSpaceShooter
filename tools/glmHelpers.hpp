#pragma once

#include <glm/glm.hpp>

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

	inline bool IsPointBetweenVectors(const glm::vec2& p, const glm::vec2& o, const glm::vec2& v1, const glm::vec2& v2)
	{
		const glm::vec2 normOP = glm::normalize(p - o);
		const glm::vec2 normV1 = glm::normalize(v1);
		const glm::vec2 normV2 = glm::normalize(v2);

		const float cross1 = glm::cross(glm::vec3(normV1, 0.0f), glm::vec3(normOP, 0.0f)).z;
		const float cross2 = glm::cross(glm::vec3(normV1, 0.0f), glm::vec3(normV2, 0.0f)).z;

		return cross1 * cross2 >= 0;
	}

	inline bool DoesVectorIntersectCircle(const glm::vec2& o, const glm::vec2& v, const glm::vec2& c, float r)
	{
		const glm::vec2 oc = o - c;
		const float a = glm::dot(v, v);
		const float b = 2.0f * glm::dot(oc, v);
		const float c_ = glm::dot(oc, oc) - r * r;

		const float delta = b * b - 4 * a * c_;

		return delta >= 0;
	}
}
