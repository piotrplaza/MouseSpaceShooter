#include "tools.hpp"

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <ios>

namespace tools
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

	}
}
