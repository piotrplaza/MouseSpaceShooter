#include <stdexcept>
#include <vector>
#include <iostream>

#include <windows.h>

#include <GL/glew.h>
#include <gl/gl.h>

#include <glm/vec2.hpp>

#include "globals.hpp"

#include "components/mouseState.hpp"
#include "components/screenInfo.hpp"
#include "components/physics.hpp"
#include "components/player.hpp"
#include "components/wall.hpp"
#include "components/grapple.hpp"
#include "components/camera.hpp"
#include "components/textureDef.hpp"

#include "systems/level.hpp"
#include "systems/player.hpp"
#include "systems/physics.hpp"
#include "systems/camera.hpp"

#include "tools/utility.hpp"

const bool fullScreen = false;
const bool console = true;
const glm::ivec2 windowRes = { 800, 800 };

void OGLInitialize()
{
	const GLenum glewInitResult = glewInit();
	assert(GLEW_OK == glewInitResult);

	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0, 0, 0, 1);
}

void CreateLevel()
{
	using namespace Globals::Components;

	//Textures.
	const unsigned spaceRockTexture = texturesDef.size();
	texturesDef.emplace_back("textures/space rock.jpg", GL_MIRRORED_REPEAT);
	const unsigned woodTexture = texturesDef.size();
	texturesDef.emplace_back("textures/wood.jpg", GL_MIRRORED_REPEAT);

	//Player configuration.
	player = Components::Player(Tools::CreateBasicPlayerBody());
	player.setPosition({ -10.0f, 0.0f });

	//Static walls.
	staticWalls.emplace_back(Tools::CreateBoxBody({ -20.0f, 0.0f }, { 0.2f, 20.0f }), woodTexture);
	staticWalls.emplace_back(Tools::CreateBoxBody({ 20.0f, 0.0f }, { 0.2f, 20.0f }), woodTexture);
	staticWalls.emplace_back(Tools::CreateBoxBody({ 0.0f, -20.0f }, { 20.0f, 0.2f }), woodTexture);
	staticWalls.emplace_back(Tools::CreateBoxBody({ 0.0f, 20.0f }, { 20.0f, 0.2f }), woodTexture);
	staticWalls.emplace_back(Tools::CreateCircleBody({ 10.0f, 0.0f }, 2.0f), spaceRockTexture);

	//Dynamic walls.
	auto& wall1 = *dynamicWalls.emplace_back(Tools::CreateBoxBody({ 5.0f, -5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f)).body;
	auto& wall2 = *dynamicWalls.emplace_back(Tools::CreateBoxBody({ 5.0f, 5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f)).body;
	wall1.GetFixtureList()->SetRestitution(1);
	wall2.GetFixtureList()->SetRestitution(1);
	Tools::PinBodies(wall1, wall2, {5.0f, 0.0f});

	//Grapples.
	grapples.emplace_back(Tools::CreateCircleBody({0.0f, 10.0f}, 1.0f), 15.0f);
	grapples.emplace_back(Tools::CreateCircleBody({ 0.0f, -10.0f }, 1.0f), 15.0f);

	//Camera.
	camera.projectionHSizeF = []() { return 25.0f + glm::length(player.getVelocity()) * 0.1f; };
	camera.projectionTransitionFactor = 0.1f;
	camera.mainActorPositionF = []() { return player.getPosition() + player.getVelocity() * 0.2f; };
	camera.positionTransitionFactor = 0.1f;
}

void Initialize()
{
	if (console) Tools::RedirectIOToConsole({ 2000, 10 });
	Tools::RandomInit();
	OGLInitialize();

	CreateLevel();

	Globals::Systems::Initialize();
}

void RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Globals::Systems::AccessLevel().render();
	Globals::Systems::AccessPlayer().render();
}

void PrepareFrame()
{
	Globals::Systems::AccessPlayer().step();
	Globals::Systems::AccessLevel().step();
	Globals::Systems::AccessCamera().step();

	RenderScene();

	Globals::Systems::AccessPhysics().step();
}

void HandleKeyboard(bool const* const keys)
{
}

void ResetMousePosition()
{
	using namespace Globals::Components;

	Tools::SetMousePos(screenInfo.windowCenterInScreenSpace);
	mouseState.position = screenInfo.windowCenterInScreenSpace;
}

void HandleMouse()
{
	using namespace Globals::Components;

	POINT mousePos;
	GetCursorPos(&mousePos);
	const auto prevPosition = mouseState.position;
	mouseState.position = { mousePos.x, mousePos.y };
	mouseState.delta = mouseState.position - prevPosition;
	
	ResetMousePosition();
}

void ChangeWindowSize(glm::ivec2 size)
{
	using namespace Globals::Components;

	screenInfo.windowSize = size;
	screenInfo.windowCenterInScreenSpace = { screenInfo.windowLocation + screenInfo.windowSize / 2 };

	glViewport(0, 0, size.x, size.y);
}

void ChangeWindowLocation(glm::ivec2 location)
{
	using namespace Globals::Components;

	screenInfo.windowLocation = location;
	screenInfo.windowCenterInScreenSpace = { location + screenInfo.windowSize / 2 };
}

void SetDCPixelFormat(HDC hDC)
{
	const PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0, 0,
		0, 0, 0, 0,
		32,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};
	const int pixelFormt = ChoosePixelFormat(hDC, &pfd);
	SetPixelFormat(hDC, pixelFormt, &pfd);
}

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
			ChangeWindowSize(size);
			break;
		}
		case WM_MOVE:
		{
			const glm::ivec2 location{ LOWORD(lParam), HIWORD(lParam) };
			ChangeWindowLocation(location);
			break;
		}
		case WM_SETFOCUS:
			ShowCursor(false);
			focus = true;
			resetMousePositionRequired = true;
			break;
		case WM_KILLFOCUS:
			ShowCursor(true);
			focus = false;
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
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0l;
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
				ResetMousePosition();
				resetMousePositionRequired = false;
			}
			HandleKeyboard(keys);
			HandleMouse();
			PrepareFrame();
			SwapBuffers(hDC);
		}
	}
	
	return (int)msg.wParam;
}
