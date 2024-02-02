#include "levels/level.hpp"
#include "levels/windmill/windmill.hpp"
#include "levels/playground/playground.hpp"
#include "levels/rocketball/rocketball.hpp"
#include "levels/gravity/gravity.hpp"
#include "levels/basic/basic.hpp"
#include "levels/dzidzia/dzidzia.hpp"
#include "levels/dzidzia2/dzidzia2.hpp"
#include "levels/rim/rim.hpp"
#include "levels/fpsScalingProblems/fpsScalingProblems.hpp"
#include "levels/squareRace/squareRace.hpp"
#include "levels/race/race.hpp"
#include "levels/splineTest/splineTest.hpp"
#include "levels/raceEditor/raceEditor.hpp"
#include "levels/collisions/collisions.hpp"
#include "levels/basic3D/basic3D.hpp"
#include "levels/firstPersonCamera/firstPersonCamera.hpp"
#include "levels/crosses3D/crosses3D.hpp"

#include "components/mouse.hpp"
#include "components/physics.hpp"
#include "components/audioListener.hpp"

#include "systems/stateController.hpp"
#include "systems/structures.hpp"
#include "systems/actors.hpp"
#include "systems/physics.hpp"
#include "systems/camera.hpp"
#include "systems/decorations.hpp"
#include "systems/temporaries.hpp"
#include "systems/cleaner.hpp"
#include "systems/deferredActions.hpp"
#include "systems/renderingController.hpp"
#include "systems/textures.hpp"
#include "systems/audio.hpp"

#include "globals/shaders.hpp"
#include "globals/components.hpp"
#include "globals/systems.hpp"

#include "ogl/oglHelpers.hpp"

#include "tools/utility.hpp"

#include <SDL.h>

#include <GL/glew.h>
#include <gl/gl.h>

#include <glm/glm.hpp>

#include <Windows.h>
#include <ShellScalingApi.h>
#include <hidusage.h>

#include <memory>
#include <stdexcept>
#include <vector>
#include <array>
#include <type_traits>

const bool fullScreen =
#ifdef _DEBUG
false;
#else 
true;
#endif
const bool console = true;
const glm::ivec2 windowRes = { 1920, 1080 };

std::unique_ptr<Levels::Level> activeLevel;

static void InitOGL()
{
	const GLenum glewInitResult = glewInit();
	assert(GLEW_OK == glewInitResult);

	Tools::VSync(true);
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_GREATER);
	//glClearDepth(0.0f);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(3.0f);
}

static void InitEngine()
{
	if (console)
		Tools::RedirectIOToConsole({ 3850, 10 });
	Tools::RandomInit();
	InitOGL();
	int sdlInitResult = SDL_Init(SDL_INIT_GAMECONTROLLER);
	assert(!sdlInitResult);

	Globals::InitializeShaders();
	Globals::InitializeComponents();
	Globals::InitializeSystems();
}

static void InitLevel()
{
#if 1
	//activeLevel = std::make_unique<Levels::RaceEditor>();
#else
	activeLevel = std::make_unique<Levels::Race>();
#endif

	//activeLevel = std::make_unique<Levels::Windmill>();
	//activeLevel = std::make_unique<Levels::SquareRace>();

	//activeLevel = std::make_unique<Levels::Playground>();
	//activeLevel = std::make_unique<Levels::Rocketball>();
	//activeLevel = std::make_unique<Levels::Gravity>();
	//activeLevel = std::make_unique<Levels::Basic>();
	//activeLevel = std::make_unique<Levels::Dzidzia>();
	//activeLevel = std::make_unique<Levels::Dzidzia2>();
	//activeLevel = std::make_unique<Levels::Rim>();
	//activeLevel = std::make_unique<Levels::SplineTest>();
	//activeLevel = std::make_unique<Levels::Collisions>();
	//activeLevel = std::make_unique<Levels::Basic3D>();
	//activeLevel = std::make_unique<Levels::FirstPersonCamera>();
	activeLevel = std::make_unique<Levels::Crosses3D>();

	//activeLevel = std::make_unique<Levels::FPSScalingProblems>();
}

static void PostInit()
{
	Globals::Systems().textures().postInit();
	Globals::Systems().physics().postInit();
	Globals::Systems().actors().postInit();
	Globals::Systems().structures().postInit();
	Globals::Systems().staticDecorations().postInit();
	Globals::Systems().camera().postInit();
	Globals::Systems().renderingController().postInit();
	Globals::Systems().audio().postInit();
	Globals::Systems().stateController().postInit();

	//Globals::Components().audioListener().setEnable(false);
}

static void PrepareFrame()
{
	if (Globals::Components().physics().paused)
	{
		Globals::Systems().physics().pause();
		Globals::Systems().camera().step();
	}
	else
	{
		activeLevel->step();

		Globals::Systems().stateController().stepSetup();
		Globals::Systems().physics().step();
		Globals::Systems().deferredActions().step();
		Globals::Systems().actors().step();
		Globals::Systems().temporaries().step();
		Globals::Systems().structures().step();
		Globals::Systems().staticDecorations().step();
		Globals::Systems().camera().step();
		Globals::Systems().audio().step();
		Globals::Systems().stateController().stepTeardown();

		Globals::Systems().cleaner().step();
	}

	Globals::Systems().stateController().renderSetup();
	Globals::Systems().renderingController().render();
	Globals::Systems().stateController().renderTeardown();
}

void SetDCPixelFormat(HDC hDC);

static std::array<bool, 256> keys;
static bool quit;
static bool focus;
static int init = 0;
static HDC hDC;

static LRESULT CALLBACK WndProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	static HGLRC hRC;

	auto dummyIfPaused = [](auto& state) -> decltype(state) {
		static std::remove_reference<decltype(state)>::type dummy{};
		return Globals::Components().physics().paused
			? dummy
			: state;
	};

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
				InitEngine();
			}
			catch (const std::runtime_error& error)
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
			if (fullScreen && init == 0) // Workaround for detecting initial, relevant WM_SIZE - in case of fullscreen, first one is messy.
			{
				++init;
				break;
			}

			const glm::ivec2 size{ LOWORD(lParam), HIWORD(lParam) };
			Globals::Systems().stateController().changeWindowSize(size);

			if (init == 0 || init == 1)
			{
				try
				{
					InitLevel();
					PostInit();
				}
				catch (const std::runtime_error& error)
				{
					MessageBox(nullptr, error.what(), "Runtime error",
						MB_OK | MB_ICONEXCLAMATION);
					ExitProcess(0);
				}
				init = 2;
			}

			break;
		}
		case WM_MOVE:
		{
			const glm::ivec2 location{ LOWORD(lParam), HIWORD(lParam) };
			Globals::Systems().stateController().changeWindowLocation(location);
			break;
		}
		case WM_SETFOCUS:
			Globals::Systems().stateController().setWindowFocus();
			ShowCursor(false);
			focus = true;
			break;
		case WM_KILLFOCUS:
			Globals::Systems().stateController().killWindowFocus();
			ShowCursor(true);
			focus = false;
			break;
		case WM_KEYDOWN:
			if (wParam == 'P' || wParam == VK_ESCAPE) keys[wParam] = true;
			else dummyIfPaused(keys[wParam]) = true;
			break;
		case WM_KEYUP:
			keys[wParam] = false;
			break;
		case WM_RBUTTONDOWN:
			dummyIfPaused(Globals::Components().mouse().pressing.rmb) = true;
			break;
		case WM_RBUTTONUP:
			Globals::Components().mouse().pressing.rmb = false;
			break;
		case WM_LBUTTONDOWN:
			dummyIfPaused(Globals::Components().mouse().pressing.lmb) = true;
			break;
		case WM_LBUTTONUP:
			Globals::Components().mouse().pressing.lmb = false;
			break;
		case WM_MBUTTONDOWN:
			dummyIfPaused(Globals::Components().mouse().pressing.mmb) = true;
			break;
		case WM_MBUTTONUP:
			Globals::Components().mouse().pressing.mmb = false;
			break;
		case WM_XBUTTONDOWN:
			switch (HIWORD(wParam))
			{
				case XBUTTON1: dummyIfPaused(Globals::Components().mouse().pressing.xmb1) = true; break;
				case XBUTTON2: dummyIfPaused(Globals::Components().mouse().pressing.xmb2) = true; break;
			}
			break;
		case WM_XBUTTONUP:
			switch (HIWORD(wParam))
			{
				case XBUTTON1: Globals::Components().mouse().pressing.xmb1 = false; break;
				case XBUTTON2: Globals::Components().mouse().pressing.xmb2 = false; break;
			}
			break;
		case WM_MOUSEWHEEL:
			if ((int)wParam > 0) ++dummyIfPaused(Globals::Components().mouse().pressing.wheel);
			else if ((int)wParam < 0) --dummyIfPaused(Globals::Components().mouse().pressing.wheel);
			break;
		case WM_INPUT:
		{
			RAWINPUT raw;
			unsigned size = sizeof(RAWINPUT);
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));

			if (raw.header.dwType == RIM_TYPEMOUSE)
			{
				dummyIfPaused(Globals::Components().mouse().delta)
					+= glm::ivec2((int)raw.data.mouse.lLastX, (int)raw.data.mouse.lLastY);
			}
			break;
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0l;
}

static void SetDCPixelFormat(HDC hDC)
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

static void RegisterRawInputDevices(HWND hWnd)
{
	RAWINPUTDEVICE rid;
	rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid.usUsage = HID_USAGE_GENERIC_MOUSE;
	rid.dwFlags = RIDEV_INPUTSINK;
	rid.hwndTarget = hWnd;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));
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

	RegisterRawInputDevices(hWnd);

	ShowWindow(hWnd, SW_SHOW);
	
	MSG msg{};

	while (!keys[VK_ESCAPE] && !quit)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (focus)
		{
			if (!Globals::Components().physics().paused)
				Globals::Systems().stateController().resetMousePosition();

			Globals::Systems().stateController().handleKeyboard(keys);
			Globals::Systems().stateController().handleMouseButtons();
			Globals::Systems().stateController().handleSDL();
			
			PrepareFrame();

			Globals::Components().mouse().delta = { 0, 0 };

			glFinish(); // Not sure why, but it helps with stuttering in some scenarios, e.g. if missile was launched (release + lower display refresh rate => bigger stuttering without it).
			SwapBuffers(hDC);
		}
	}
	
	return (int)msg.wParam;
}
