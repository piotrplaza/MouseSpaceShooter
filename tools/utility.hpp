#pragma once

#include <glm/vec2.hpp>

namespace tools
{
	void RedirectIOToConsole(glm::ivec2 windowPos = {10, 10}, int maxConsoleLines = 500);

	glm::vec2 GetNormalizedMousePosition();
	void SetMousePos(glm::ivec2 mousePos);
	void SetMouseCursorVisibility(bool visibility);

	void RandomInit();
	float Random(float min, float max);
}
