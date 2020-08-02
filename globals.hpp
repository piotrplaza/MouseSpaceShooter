#pragma once

#include <vector>

namespace Components
{
	struct MouseState;
	struct ScreenInfo;
	struct MVP;
	struct Physics;
	struct TextureDef;
	struct Texture;
	struct Player;
	struct Wall;
	struct Grapple;
	struct Connection;
	struct Camera;
	struct Decoration;
	struct GraphicsSettings;
}

namespace Systems
{
	class StateController;
	class Physics;
	class Textures;
	class Players;
	class Level;
	class Camera;
	class Decorations;
}

namespace Globals
{
	namespace Components
	{
		extern ::Components::MouseState& mouseState;
		extern ::Components::ScreenInfo& screenInfo;
		extern ::Components::MVP& mvp;
		extern ::Components::Physics& physics;
		extern ::Components::Camera& camera;
		extern ::Components::GraphicsSettings& graphicsSettings;

		extern std::vector<::Components::TextureDef>& texturesDef;
		extern std::vector<::Components::Texture>& textures;
		extern std::vector<::Components::Player>& players;
		extern std::vector<::Components::Wall>& staticWalls;
		extern std::vector<::Components::Wall>& dynamicWalls;
		extern std::vector<::Components::Grapple>& grapples;
		extern std::vector<::Components::Connection>& connections;
		extern std::vector<::Components::Decoration>& backgroundDecorations;
		extern std::vector<::Components::Decoration>& midgroundDecorations;
		extern std::vector<::Components::Decoration>& foregroundDecorations;
	}

	namespace Systems
	{
		void Initialize();

		::Systems::StateController& AccessStateController();
		::Systems::Physics& AccessPhysics();
		::Systems::Textures& AccessTextures();
		::Systems::Players& AccessPlayers();
		::Systems::Level& AccessLevel();
		::Systems::Camera& AccessCamera();
		::Systems::Decorations& AccessDecorations();
	}

	namespace Constants
	{
		constexpr float playerForwardForce = 15.0f;
		constexpr float playerLinearDamping = 0.1f;
		constexpr float playerAngularDamping = 15.0f;

		constexpr float mouseSensitivity = 0.01f;
		constexpr float targetFrameTime = 1.0f / 60;
		constexpr int circleGraphicsComplexity = 60;
		constexpr int maxTextureObjects = 100;
	}
}
