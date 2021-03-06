#include <memory>
#include <stdexcept>
#include <vector>

#include <windows.h>
#include <shellscalingapi.h>

#include <GL/glew.h>
#include <gl/gl.h>

#include <glm/glm.hpp>

#include "globals.hpp"

#include "components/mouseState.hpp"
#include "components/screenInfo.hpp"
#include "components/graphicsSettings.hpp"
#include "components/lowResBuffers.hpp"

#include "systems/stateController.hpp"
#include "systems/walls.hpp"
#include "systems/players.hpp"
#include "systems/physics.hpp"
#include "systems/camera.hpp"
#include "systems/decorations.hpp"
#include "systems/temporaries.hpp"
#include "systems/cleaner.hpp"
#include "systems/deferredActions.hpp"

#include "levels/level.hpp"
#include "levels/playground/playground.hpp"
#include "levels/rocketball/rocketball.hpp"
#include "levels/race1/race1.hpp"

#include "ogl/oglHelpers.hpp"

#include <globals.hpp>
#include <components/shockwave.hpp>

const bool fullScreen = true;
const bool console = true;
const glm::ivec2 windowRes = { 1600, 1600 };

std::unique_ptr<Levels::Level> activeLevel;

void OGLInitialize()
{
	const GLenum glewInitResult = glewInit();
	assert(GLEW_OK == glewInitResult);

	Tools::VSync(true);
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(3.0f);
}

void CreateLevel()
{
	activeLevel = std::make_unique<Levels::Playground>();
	//activeLevel = std::make_unique<Levels::Rocketball>();
	//activeLevel = std::make_unique<Levels::Race1>();
}

void Initialize()
{
	if (console) Tools::RedirectIOToConsole({ 2000, 10 });
	Tools::RandomInit();
	OGLInitialize();

	CreateLevel();

	Globals::Systems::Initialize();

	Globals::Systems::StateController().initializationFinalize();
}

void RenderScene()
{
	using namespace Globals::Components;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenInfo.windowSize.x, screenInfo.windowSize.y);
	const glm::vec4& clearColor = Globals::Components::graphicsSettings.clearColor;
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Globals::Systems::Decorations().renderBackground();
	Globals::Systems::Decorations().renderFarMidground();
	Globals::Systems::Walls().render();
	Globals::Systems::Players().render();
	Globals::Systems::Temporaries().render();
	Globals::Systems::Decorations().renderNearMidground();
	Globals::Systems::Decorations().renderForeground();

	/*glPointSize(10);
	for (const auto& shockwave : Globals::Components::shockwaves)
	for (const auto& particle : shockwave.second.particles)
	{
		glBegin(GL_POINTS);
		glVertex2fv(&particle->GetWorldCenter().x);
		glEnd();
	}*/

	/*static bool first = true;
	static const GLuint list = glGenLists(1);
	if (first)
	{
		glNewList(list, GL_COMPILE);
			const float r = 1.0f;
			glBegin(GL_TRIANGLES);
			for (int i = 0; i < 10000; ++i)
				for (int j = 0; j < 3; ++j)
				{
					const float angle = Tools::Random(0.0f, glm::two_pi<float>());
					glVertex2f(glm::cos(angle) * r, glm::sin(angle) * r);
				}
			glEnd();
		glEndList();
		first = false;
	}
	glCallList(list);*/
}

void PrepareFrame()
{
	Globals::Systems::StateController().frameSetup();

	Globals::Systems::Physics().step();
	Globals::Systems::DeferredActions().step();

	activeLevel->step();

	Globals::Systems::Players().step();
	Globals::Systems::Temporaries().step();
	Globals::Systems::Walls().step();
	Globals::Systems::Decorations().step();
	Globals::Systems::Camera().step();

	RenderScene();

	Globals::Systems::StateController().frameTeardown();

	Globals::Systems::Cleaner().step();
}

void SetDCPixelFormat(HDC hDC);

static bool keys[256];
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
	using namespace Globals::Components;

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
			Globals::Systems::StateController().changeWindowSize(size);
			break;
		}
		case WM_MOVE:
		{
			const glm::ivec2 location{ LOWORD(lParam), HIWORD(lParam) };
			Globals::Systems::StateController().changeWindowLocation(location);
			break;
		}
		case WM_SETFOCUS:
			ShowCursor(false);
			focus = true;
			resetMousePositionRequired = true;
			Globals::Systems::Physics().resume();
			break;
		case WM_KILLFOCUS:
			ShowCursor(true);
			focus = false;
			Globals::Systems::Physics().pause();
			break;
		case WM_KEYDOWN:
			keys[wParam] = true;
			break;
		case WM_KEYUP:
			keys[wParam] = false;
			break;
		case WM_RBUTTONDOWN:
			mouseState.rmb = true;
			break;
		case WM_RBUTTONUP:
			mouseState.rmb = false;
			break;
		case WM_LBUTTONDOWN:
			mouseState.lmb = true;
			break;
		case WM_LBUTTONUP:
			mouseState.lmb = false;
			break;
		case WM_MBUTTONDOWN:
			mouseState.mmb = true;
			break;
		case WM_MBUTTONUP:
			mouseState.mmb = false;
			break;
		case WM_XBUTTONDOWN:
			switch (HIWORD(wParam))
			{
				case XBUTTON1: mouseState.xmb1 = true; break;
				case XBUTTON2: mouseState.xmb2 = true; break;
			}
			break;
		case WM_XBUTTONUP:
			switch (HIWORD(wParam))
			{
				case XBUTTON1: mouseState.xmb1 = false; break;
				case XBUTTON2: mouseState.xmb2 = false; break;
			}
			break;
		case WM_MOUSEWHEEL:
			if ((int)wParam > 0) ++mouseState.wheel;
			else if ((int)wParam < 0) --mouseState.wheel;
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
				Globals::Systems::StateController().resetMousePosition();
				resetMousePositionRequired = false;
			}
			Globals::Systems::StateController().handleKeyboard(keys);
			Globals::Systems::StateController().handleMousePosition();
			PrepareFrame();

			glFinish(); //Not sure why, but it helps with stuttering in some scenarios, e.g. if missile was launched (release + lower display refresh rate => bigger stuttering without it).
			SwapBuffers(hDC);
		}
	}
	
	return (int)msg.wParam;
}
