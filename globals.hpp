#pragma once

#include <vector>

namespace Components
{
	struct MouseState;
	struct ScreenInfo;
	struct MVP;
	struct Physics;
	struct Player;
	struct Wall;
	struct Grapple;
	struct Connection;
	struct Camera;
	struct Texture;
}

namespace Systems
{
	class Physics;
	class Player;
	class Level;
	class Camera;
	class Textures;
}

namespace Globals
{
	namespace Components
	{
		extern ::Components::MouseState& mouseState;
		extern ::Components::ScreenInfo& screenInfo;
		extern ::Components::MVP& mvp;
		extern ::Components::Physics& physics;
		extern ::Components::Player& player;
		extern ::Components::Camera& camera;

		extern std::vector<::Components::Wall>& staticWalls;
		extern std::vector<::Components::Wall>& dynamicWalls;
		extern std::vector<::Components::Grapple>& grapples;
		extern std::vector<::Components::Connection>& connections;
		extern std::vector<::Components::Texture>& textures;
	}

	namespace Systems
	{
		void Initialize();

		::Systems::Physics& AccessPhysics();
		::Systems::Player& AccessPlayer();
		::Systems::Level& AccessLevel();
		::Systems::Camera& AccessCamera();
		::Systems::Textures& AccessTextures();
	}

	namespace Defaults
	{
		constexpr float playerForwardForce = 15.0f;
		constexpr float playerLinearDamping = 0.1f;
		constexpr float playerAngularDamping = 15.0f;
		constexpr float playerAutoRotationFactor = 0.5f;

		constexpr float mouseSensitivity = 0.01f;
		constexpr int circleGraphicsComplexity = 20;
	}
}
