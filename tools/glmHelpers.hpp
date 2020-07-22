#pragma once

#include <glm/vec2.hpp>

inline glm::vec2 operator +(const glm::vec2& lhs, const glm::ivec2& rhs)
{
	return { lhs.x + rhs.x, lhs.y + rhs.y };
}

inline glm::vec2 operator *(const glm::ivec2& lhs, const glm::vec2& rhs)
{
	return { lhs.x * rhs.x, lhs.y * rhs.y };
}

inline glm::vec2 operator /(const glm::vec2& lhs, const glm::ivec2& rhs)
{
	return { lhs.x / rhs.x, lhs.y / rhs.y };
}
