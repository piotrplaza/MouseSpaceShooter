#include "levels/level.hpp"
#include "levels/playground/playground.hpp"
#include "levels/rocketball/rocketball.hpp"
#include "levels/gravity/gravity.hpp"

#include "components/mouseState.hpp"
#include "components/physics.hpp"

#include "systems/stateController.hpp"
#include "systems/walls.hpp"
#include "systems/players.hpp"
#include "systems/physics.hpp"
#include "systems/camera.hpp"
#include "systems/decorations.hpp"
#include "systems/temporaries.hpp"
#include "systems/cleaner.hpp"
#include "systems/deferredActions.hpp"
#include "systems/renderingController.hpp"
#include "systems/textures.hpp"

#include "globals/shaders.hpp"
#include "globals/components.hpp"
#include "globals/systems.hpp"

#include "ogl/oglHelpers.hpp"

#include "tools/utility.hpp"

#include <GL/glew.h>
#include <gl/gl.h>

#include <glm/glm.hpp>

#include <memory>
#include <stdexcept>
#include <vector>
#include <array>

#include <windows.h>
#include <shellscalingapi.h>

const bool fullScreen =
#ifdef _DEBUG
false;
#else 
true;
#endif
const bool console = true;
const glm::ivec2 windowRes = { 1920, 1080 };

std::unique_ptr<Levels::Level> activeLevel;

void OGLInitialize()
{
	const GLenum glewInitResult = glewInit();
	assert(GLEW_OK == glewInitResult);

	Tools::VSync(true);
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(5.0f);
}

void CreateLevel()
{
	activeLevel = std::make_unique<Levels::Playground>();
	//activeLevel = std::make_unique<Levels::Rocketball>();
	//activeLevel = std::make_unique<Levels::Gravity>();
}

void Initialize()
{
	if (console) Tools::RedirectIOToConsole({ 4000, 10 });
	Tools::RandomInit();
	OGLInitialize();

	Globals::InitializeShaders();
	Globals::InitializeComponents();
	Globals::InitializeSystems();

	CreateLevel();
	Globals::Systems().textures().postInit();
	Globals::Systems().physics().postInit();
	Globals::Systems().players().postInit();
	Globals::Systems().walls().postInit();
	Globals::Systems().decorations().postInit();
	Globals::Systems().camera().postInit();
	Globals::Systems().renderingController().postInit();
	Globals::Systems().stateController().postInit();
}

void PrepareFrame()
{
	Globals::Systems().stateController().frameSetup();
	Globals::Systems().physics().step();
	Globals::Systems().deferredActions().step();
	Globals::Systems().players().step();
	Globals::Systems().temporaries().step();
	Globals::Systems().walls().step();
	Globals::Systems().decorations().step();
	Globals::Systems().camera().step();

	activeLevel->step();
	Globals::Systems().stateController().renderSetup();
	Globals::Systems().renderingController().render();
	Globals::Systems().stateController().frameTeardown();
	Globals::Systems().cleaner().step();
}

void SetDCPixelFormat(HDC hDC);

static std::array<bool, 256> keys;
static bool quit;
static bool focus;
static bool resetMousePositionRequired;
static HDC hDC;

LRESULT CALLBACK WndProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	static HGLRC hRC;

	switch(message)
	{
		case WM_CREATE:
		{
			hDC = GetDC(hWnd);
			SetDCPixelFormat(hDC);
			hRC = wglCreateContext(hDC);
			wglMakeCurrent(hDC, hRC);
			try
			{
				Initialize();
			}
			catch (const std::runtime_error & error)
			{
				MessageBox(nullptr, error.what(), "Runtime error",
					MB_OK | MB_ICONEXCLAMATION);
				ExitProcess(0);
			}
			break;
		}
		case WM_DESTROY:
			wglMakeCurrent(hDC, nullptr);
			wglDeleteContext(hRC);
			PostQuitMessage(0);
			quit = true;
			break;
		case WM_SIZE:
		{
			const glm::ivec2 size{ LOWORD(lParam), HIWORD(lParam) };
			Globals::Systems().stateController().changeWindowSize(size);
			break;
		}
		case WM_MOVE:
		{
			const glm::ivec2 location{ LOWORD(lParam), HIWORD(lParam) };
			Globals::Systems().stateController().changeWindowLocation(location);
			break;
		}
		case WM_SETFOCUS:
			ShowCursor(false);
			focus = true;
			resetMousePositionRequired = true;
			Globals::Components().physics().paused = false;
			break;
		case WM_KILLFOCUS:
			ShowCursor(true);
			focus = false;
			Globals::Components().physics().paused = true;
			break;
		case WM_KEYDOWN:
			keys[wParam] = true;
			break;
		case WM_KEYUP:
			keys[wParam] = false;
			break;
		case WM_RBUTTONDOWN:
			Globals::Components().mouseState().rmb = true;
			break;
		case WM_RBUTTONUP:
			Globals::Components().mouseState().rmb = false;
			break;
		case WM_LBUTTONDOWN:
			Globals::Components().mouseState().lmb = true;
			break;
		case WM_LBUTTONUP:
			Globals::Components().mouseState().lmb = false;
			break;
		case WM_MBUTTONDOWN:
			Globals::Components().mouseState().mmb = true;
			break;
		case WM_MBUTTONUP:
			Globals::Components().mouseState().mmb = false;
			break;
		case WM_XBUTTONDOWN:
			switch (HIWORD(wParam))
			{
				case XBUTTON1: Globals::Components().mouseState().xmb1 = true; break;
				case XBUTTON2: Globals::Components().mouseState().xmb2 = true; break;
			}
			break;
		case WM_XBUTTONUP:
			switch (HIWORD(wParam))
			{
				case XBUTTON1: Globals::Components().mouseState().xmb1 = false; break;
				case XBUTTON2: Globals::Components().mouseState().xmb2 = false; break;
			}
			break;
		case WM_MOUSEWHEEL:
			if ((int)wParam > 0) ++Globals::Components().mouseState().wheel;
			else if ((int)wParam < 0) --Globals::Components().mouseState().wheel;
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0l;
}

void SetDCPixelFormat(HDC hDC)
{
	const PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0,
		0, 0,
		0, 0, 0, 0,
		24,
		8,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};
	const int pixelFormt = ChoosePixelFormat(hDC, &pfd);
	SetPixelFormat(hDC, pixelFormt, &pfd);
}

int APIENTRY WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	const LPCTSTR lpszAppName = "OpenGL window";
	const int winPosX = 10, winPosY = 10;

	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	WNDCLASS wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = nullptr;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = lpszAppName;
	
	if(!RegisterClass(&wc))
	{
		MessageBox(nullptr, "Window class registration failed.", "Error",
			MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	HWND hWnd = CreateWindow(
		lpszAppName,
		lpszAppName,
		fullScreen ? WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP | WS_MAXIMIZE : 
		WS_OVERLAPPEDWINDOW,
		winPosX, winPosY,
		windowRes.x, windowRes.y,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	if(!hWnd)
	{
		MessageBox(nullptr, "Window creation failed.", "Error",
			MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	ShowWindow(hWnd, SW_SHOW);
	
	MSG msg{};

	while (!keys[VK_ESCAPE] && !quit)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if(focus)
		{
			if (resetMousePositionRequired)
			{
				Globals::Systems().stateController().resetMousePosition();
				resetMousePositionRequired = false;
			}
			Globals::Systems().stateController().handleKeyboard(keys);
			Globals::Systems().stateController().handleMousePosition();
			PrepareFrame();

			glFinish(); //Not sure why, but it helps with stuttering in some scenarios, e.g. if missile was launched (release + lower display refresh rate => bigger stuttering without it).
			SwapBuffers(hDC);
		}
	}
	
	return (int)msg.wParam;
}
