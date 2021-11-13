#pragma once

#include <glm/vec2.hpp>

namespace Tools
{
	void RedirectIOToConsole(glm::ivec2 windowPos = {10, 10}, int maxConsoleLines = 500);

	glm::vec2 GetNormalizedMousePosition();
	void SetMousePos(glm::ivec2 mousePos);
	glm::ivec2 GetMousePos();
	void SetMouseCursorVisibility(bool visibility);

	void RandomInit();
	float Random(float min, float max);
	unsigned StableRandom(unsigned seed);
	float StableRandom(float min, float max, unsigned seed);

	template <class T>
	inline void HashCombine(std::size_t& seed, const T& v)
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
}
