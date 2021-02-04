#include "utility.hpp"

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <ios>
#include <cstdlib>
#include <ctime>
#include <climits>

namespace Tools
{
	void RedirectIOToConsole(glm::ivec2 windowPos, int maxConsoleLines)
	{
		CONSOLE_SCREEN_BUFFER_INFO coninfo;

		AllocConsole();

		(void)freopen("CONIN$", "r", stdin);
		(void)freopen("CONOUT$", "w", stdout);
		(void)freopen("CONOUT$", "w", stderr);

		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
		coninfo.dwSize.Y = maxConsoleLines;
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

		HWND consoleWindow = GetConsoleWindow();
		SetWindowPos(consoleWindow, 0, windowPos.x, windowPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	glm::vec2 GetNormalizedMousePosition()
	{
		return {};
	}

	void SetMousePos(glm::ivec2 mousePos)
	{
		SetCursorPos(mousePos.x, mousePos.y);
	}

	void SetMouseCursorVisibility(bool visibility)
	{
		ShowCursor(visibility);
	}

	void RandomInit()
	{
		std::srand((unsigned int)std::time(NULL));
	}

	float Random(float min, float max)
	{
		return (float)std::rand() / RAND_MAX * (max - min) + min;
	}

	unsigned StableRandom(unsigned seed)
	{
		unsigned i = (seed ^ 12345391u) * 2654435769u;
		i ^= (i << 6u) ^ (i >> 26u);
		i *= 2654435769u;
		i += (i << 5u) ^ (i >> 12u);
		return i;
	}

	float StableRandom(float min, float max, unsigned seed)
	{
		return (float)StableRandom(seed) / UINT_MAX * (max - min) + min;
	}
}
