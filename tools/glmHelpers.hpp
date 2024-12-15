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
