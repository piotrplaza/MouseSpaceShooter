#include "levels/level.hpp"
#include "levels/windmill/windmill.hpp"
#include "levels/squareRace/squareRace.hpp"
#include "levels/snakeCube/snakeCube.hpp"
#include "levels/playground/playground.hpp"
#include "levels/rocketball/rocketball.hpp"
#include "levels/gravity/gravity.hpp"
#include "levels/basic/basic.hpp"
#include "levels/dzidzia/dzidzia.hpp"
#include "levels/dzidzia2/dzidzia2.hpp"
#include "levels/rim/rim.hpp"
#include "levels/fpsScalingProblems/fpsScalingProblems.hpp"
#include "levels/race/race.hpp"
#include "levels/splineTest/splineTest.hpp"
#include "levels/raceEditor/raceEditor.hpp"
#include "levels/collisions/collisions.hpp"
#include "levels/basic3D/basic3D.hpp"
#include "levels/firstPersonCamera/firstPersonCamera.hpp"
#include "levels/crosses3D/crosses3D.hpp"
#include "levels/crosses3DTexturing/crosses3DTexturing.hpp"
#include "levels/crosses3DInstancing/crosses3DInstancing.hpp"
#include "levels/paint/paint.hpp"
#include "levels/noise/noise.hpp"
#include "levels/decals/decals.hpp"
#include "levels/particles2D/particles2D.hpp"
#include "levels/particles3D/particles3D.hpp"
#include "levels/tests/tests.hpp"

#include "levels/damageOn/nest/nest.hpp"

#include "components/mouse.hpp"
#include "components/physics.hpp"
#include "components/audioListener.hpp"
#include "components/appStateHandler.hpp"

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

#include <ogl/oglProxy.hpp>

#include <glm/glm.hpp>

#include <Windows.h>
#include <ShellScalingApi.h>
#include <hidusage.h>

#include <memory>
#include <stdexcept>
#include <vector>
#include <array>
#include <type_traits>
#include <thread>

const bool debugFullscreen = false;
const bool releaseFullscreen = true;
const bool devFullScreen = true;
const bool console = true;
const bool glDebug = true;
const bool audio = true;
const GLenum glDebugMinSeverity = GL_DEBUG_SEVERITY_LOW;
const glm::ivec2 windowRes = { 800, 600 };
const glm::ivec2 windowPos = { 10, 10 };
const glm::ivec2 consolePos = { 850, 10 };

const struct
{
	glm::ivec2 resolution;
	int bitsPerPixel;
	int refreshRate;
	bool enabled;
} forcedScreenMode = { { 800, 600 }, 32, 60, false };

const bool fullScreen =
#ifdef _DEBUG
debugFullscreen;
#else 
releaseFullscreen;
#endif

std::unique_ptr<Levels::Level> activeLevel;

static void InitOGL()
{
	const GLenum glewInitResult = glewInit();
	assert(GLEW_OK == glewInitResult);

	if (glDebug)
		glProxyEnableDebugOutput(glDebugMinSeverity);

	Tools::VSync(true);
	glProxySetBlend(true);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

static void InitSDL()
{
	int sdlInitResult = SDL_Init(SDL_INIT_GAMECONTROLLER);
	assert(!sdlInitResult);
}

static void InitEngine()
{
	Tools::RandomInit();

	InitOGL();
	InitSDL();

	Globals::InitializeShaders();
	Globals::InitializeComponents();
	Globals::InitializeSystems();
}

static void InitLevel()
{
	//activeLevel = std::make_unique<Levels::RaceEditor>();
	//activeLevel = std::make_unique<Levels::Race>();

	//activeLevel = std::make_unique<Levels::Windmill>();
	//activeLevel = std::make_unique<Levels::SquareRace>();
	//activeLevel = std::make_unique<Levels::SnakeCube>();

	//activeLevel = std::make_unique<Levels::DamageOn::Nest>();

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
	//activeLevel = std::make_unique<Levels::Crosses3D>();
	//activeLevel = std::make_unique<Levels::Crosses3DTexturing>();
	//activeLevel = std::make_unique<Levels::Crosses3DInstancing>();
	//activeLevel = std::make_unique<Levels::Paint>();
	//activeLevel = std::make_unique<Levels::Noise>();
	//activeLevel = std::make_unique<Levels::Decals>();
	activeLevel = std::make_unique<Levels::Particles2D>();
	//activeLevel = std::make_unique<Levels::Particles3D>();
	//activeLevel = std::make_unique<Levels::Tests>();

	//activeLevel = std::make_unique<Levels::FPSScalingProblems>();
}

static void PostInit()
{
	Globals::Components().audioListener().forceDisabled(!audio);

	Globals::Systems().stateController().postInit();
	Globals::Systems().physics().postInit();
	Globals::Systems().camera().postInit();
	Globals::Systems().textures().postInit();

	activeLevel->postSetup();

	Globals::Systems().actors().postInit();
	Globals::Systems().structures().postInit();
	Globals::Systems().decorations().postInit();
	Globals::Systems().renderingController().postInit();
	Globals::Systems().audio().postInit();
}

static void PrepareFrame()
{
	if (Globals::Components().physics().paused)
	{
		Globals::Systems().stateController().stepSetup();
		Globals::Systems().physics().step();
		Globals::Systems().camera().step();
	}
	else
	{
		Globals::Systems().stateController().stepSetup();
		Globals::Systems().physics().step();
		Globals::Systems().deferredActions().step();

		activeLevel->step();

		Globals::Systems().textures().step();
		Globals::Systems().actors().step();
		Globals::Systems().temporaries().step();
		Globals::Systems().structures().step();
		Globals::Systems().decorations().step();
		Globals::Systems().camera().step();
		Globals::Systems().audio().step();
		Globals::Systems().stateController().stepTeardown();
	}

	Globals::Systems().stateController().renderSetup();
	Globals::Systems().renderingController().render();
	Globals::Systems().stateController().renderTeardown();

	if (!Globals::Components().physics().paused)
		Globals::Systems().cleaner().step();

	//std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

static void TearDown()
{
	activeLevel.reset();

	Globals::TeardownSystems();
	Globals::DestroyComponents();
	Globals::DestroySystems();
	Globals::DestroyShaders();

	SDL_Quit();
}

void SetDCPixelFormat(HDC hDC);

static std::array<bool, 256> keys;
static bool quit;
static bool focus;
static bool prevFocus;
static bool first = true;
static std::optional<glm::ivec2> newPos;
static std::optional<glm::ivec2> newSize;
static int init = 0;
static HDC hDC;

static LRESULT CALLBACK WndProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	static HGLRC hRC;

	if (wParam == SC_KEYMENU)
		return 0;

	auto isAltGrActive = []() {
		return (GetKeyState(VK_RMENU) & 0x8000) && (GetKeyState(VK_CONTROL) & 0x8000);
	};

	switch(message)
	{
		case WM_CREATE:
		{
			hDC = GetDC(hWnd);
			SetDCPixelFormat(hDC);
			hRC = wglCreateContext(hDC);
			wglMakeCurrent(hDC, hRC);
			break;
		}
		case WM_DESTROY:
			wglMakeCurrent(hDC, nullptr);
			wglDeleteContext(hRC);
			PostQuitMessage(0);
			quit = true;
			break;
		case WM_SIZE:
			newSize = { LOWORD(lParam), HIWORD(lParam) };
			break;
		case WM_MOVE:
			newPos = { LOWORD(lParam), HIWORD(lParam) };
			break;
		case WM_SETFOCUS:
			focus = true;
			break;
		case WM_KILLFOCUS:
			focus = false;
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			keys[wParam] = true;
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (wParam == VK_CONTROL && isAltGrActive())
				break;
			keys[wParam] = false;
			break;
		case WM_RBUTTONDOWN:
			Globals::Components().mouse().pressing.rmb = true;
			break;
		case WM_RBUTTONUP:
			Globals::Components().mouse().pressing.rmb = false;
			break;
		case WM_LBUTTONDOWN:
			Globals::Components().mouse().pressing.lmb = true;
			break;
		case WM_LBUTTONUP:
			Globals::Components().mouse().pressing.lmb = false;
			break;
		case WM_MBUTTONDOWN:
			Globals::Components().mouse().pressing.mmb = true;
			break;
		case WM_MBUTTONUP:
			Globals::Components().mouse().pressing.mmb = false;
			break;
		case WM_XBUTTONDOWN:
			switch (HIWORD(wParam))
			{
				case XBUTTON1: Globals::Components().mouse().pressing.xmb1 = true; break;
				case XBUTTON2: Globals::Components().mouse().pressing.xmb2 = true; break;
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
			if ((int)wParam > 0) ++Globals::Components().mouse().pressing.wheel;
			else if ((int)wParam < 0) --Globals::Components().mouse().pressing.wheel;
			break;
		case WM_INPUT:
		{
			RAWINPUT raw{};
			unsigned size = sizeof(RAWINPUT);
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));

			if (raw.header.dwType == RIM_TYPEMOUSE)
			{
				Globals::Components().mouse().delta += glm::ivec2((int)raw.data.mouse.lLastX, (int)raw.data.mouse.lLastY);
			}
			break;
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
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
	RAWINPUTDEVICE rid{};
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
	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	if (console)
		Tools::RedirectIOToConsole(consolePos);

	if (forcedScreenMode.enabled)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = forcedScreenMode.resolution.x;
		dmScreenSettings.dmPelsHeight = forcedScreenMode.resolution.y;
		dmScreenSettings.dmBitsPerPel = forcedScreenMode.bitsPerPixel;
		dmScreenSettings.dmDisplayFrequency = forcedScreenMode.refreshRate;
		dmScreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			MessageBox(NULL, "Unable to change to selected full screen mode.", "Error", MB_OK);
			return 1;
		}
	}

	const LPCTSTR lpszAppName = "OpenGL window";

	WNDCLASS wc{};
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
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
		fullScreen ? WS_POPUP | WS_MAXIMIZE :
		WS_OVERLAPPEDWINDOW,
		windowPos.x, windowPos.y,
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

	if (fullScreen && devFullScreen) // It makes the full screen borderless window appear faster, but on some systems it's not as smooth as the other way.
		SetWindowLong(hWnd, GWL_STYLE, 0);

	RegisterRawInputDevices(hWnd);
	ShowWindow(hWnd, SW_SHOW);

	MSG msg{};

	try
	{
		InitEngine();

		while (!Globals::Components().appStateHandler().exitF() && !quit)
		{
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (newPos)
			{
				Globals::Systems().stateController().changeWindowLocation(*newPos);
				Globals::Systems().stateController().changeRefreshRate(GetDeviceCaps(hDC, VREFRESH));
				newPos = {};
			}

			if (newSize)
			{
				Globals::Systems().stateController().changeWindowSize(*newSize);
				newSize = {};
			}

			if (prevFocus != focus)
			{
				keys = {};
				Globals::Systems().stateController().setWindowFocus(focus);
				prevFocus = focus;
			}

			if (first)
			{
				InitLevel();
				PostInit();
				first = false;
			}

			if (focus)
			{
				if (!Globals::Components().physics().paused)
				{
					Tools::SetMouseCursorVisibility(false);
					Globals::Systems().stateController().resetMousePosition();
				}
				else
					Tools::SetMouseCursorVisibility(true);

				Globals::Systems().stateController().handleKeyboard(keys);
				Globals::Systems().stateController().handleMouseButtons();
				Globals::Systems().stateController().handleSDL();

				PrepareFrame();

				Globals::Components().mouse().delta = { 0, 0 };

				glFinish(); // Not sure why, but it helps with stuttering in some scenarios, e.g. if projectile was launched (release + lower display refresh rate => bigger stuttering without it).
				//GdiFlush();
				SwapBuffers(hDC);
			}
			else
				Tools::SetMouseCursorVisibility(true);
		}

		TearDown();
	}
	catch (const std::runtime_error& error)
	{
		Tools::SetMouseCursorVisibility(true);
		MessageBox(nullptr, error.what(), "Runtime error",
			MB_OK | MB_ICONEXCLAMATION);
	}
	
	return (int)msg.wParam;
}
