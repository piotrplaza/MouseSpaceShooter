#include "utility.hpp"

#include <Windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <ios>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <iostream>

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

	void PrintWarning(const std::string& warning)
	{
#ifdef _DEBUG
		std::cout << "Warning: " << warning << std::endl;
#endif
	}

	void PrintError(const std::string& error)
	{
		std::cout << "Error: " << error << std::endl;
	}

	glm::vec2 GetNormalizedMousePosition()
	{
		return {};
	}

	void SetMousePos(glm::ivec2 mousePos)
	{
		SetCursorPos(mousePos.x, mousePos.y);
	}

	glm::ivec2 GetMousePos()
	{
		POINT mousePos;
		GetCursorPos(&mousePos);
		return { mousePos.x, mousePos.y };
	}

	void SetMouseCursorVisibility(bool visibility)
	{
		static bool cursorVisibility = true;
		if (cursorVisibility == visibility)
			return;

		ShowCursor(cursorVisibility = visibility);
	}

	void RandomInit()
	{
		std::srand((unsigned int)std::time(NULL));
	}

	float RandomFloat(float min, float max)
	{
		return (float)std::rand() / RAND_MAX * (max - min) + min;
	}

	int RandomInt(int min, int max)
	{
		return std::rand() % (max - min + 1) + min;
	}

	unsigned FastStableRandom(unsigned seed)
	{
		unsigned i = (seed ^ 12345391u) * 2654435769u;
		i ^= (i << 6u) ^ (i >> 26u);
		i *= 2654435769u;
		i += (i << 5u) ^ (i >> 12u);
		return i;
	}

	float FastStableRandom(float min, float max, unsigned seed)
	{
		return (float)FastStableRandom(seed) / UINT_MAX * (max - min) + min;
	}

	float ApplyDeadzone(float input, float deadzone)
	{
		return std::abs(input) < deadzone
			? 0.0f
			: input;
	}

	glm::vec2 ApplyDeadzone(glm::vec2 input, float deadzone, bool axesSeparation)
	{
		if (axesSeparation)
			return { std::abs(input.x) < deadzone
				? 0.0f
				: input.x,
				std::abs(input.y) < deadzone
				? 0.0f
				: input.y
			};
		else
			return std::abs(input.x) < deadzone && std::abs(input.y) < deadzone
				? glm::vec2(0.0f)
				: input;
	}

	int StableRandom::Hash(int x)
	{
		x += (x << 10u);
		x ^= (x >> 6u);
		x += (x << 3u);
		x ^= (x >> 11u);
		x += (x << 15u);
		return x;
	}

	int StableRandom::Hash(glm::ivec2 v)
	{
		return Hash(v.x ^ Hash(v.y));
	}

	int StableRandom::Hash(glm::ivec3 v)
	{
		return Hash(v.x ^ Hash(v.y) ^ Hash(v.z));
	}

	int StableRandom::Hash(glm::ivec4 v)
	{
		return Hash(v.x ^ Hash(v.y) ^ Hash(v.z) ^ Hash(v.w));
	}

	int StableRandom::Hash(int x, int seed)
	{
		return Hash(x ^ Hash(seed));
	}

	int StableRandom::Hash(glm::ivec2 v, int seed)
	{
		return Hash(v.x ^ Hash(v.y) ^ Hash(seed));
	}

	int StableRandom::Hash(glm::ivec3 v, int seed)
	{
		return Hash(v.x ^ Hash(v.y) ^ Hash(v.z) ^ Hash(seed));
	}

	int StableRandom::Hash(glm::ivec4 v, int seed)
	{
		return Hash(v.x ^ Hash(v.y) ^ Hash(v.z) ^ Hash(v.w) ^ Hash(seed));
	}
}
