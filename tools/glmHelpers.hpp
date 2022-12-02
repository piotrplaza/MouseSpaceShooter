#pragma once

#include <glm/vec2.hpp>

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

inline std::vector<glm::vec3> convertToVec3Vector(const std::vector<glm::vec2>& sourceVec)
{
	std::vector<glm::vec3> result;
	result.reserve(sourceVec.size());
	for (const auto e : sourceVec)
		result.emplace_back(e, 0.0f);
	return result;
}
