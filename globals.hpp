#pragma once

#include <vector>
#include <unordered_map>

#include "componentId.hpp"

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
	struct Missile;
	struct CollisionHandler;
}

namespace Systems
{
	class StateController;
	class Physics;
	class Textures;
	class Players;
	class Walls;
	class Camera;
	class Decorations;
	class Temporaries;
	class Cleaner;
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
		extern std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryBackgroundDecorations;
		extern std::vector<::Components::Decoration>& midgroundDecorations;
		extern std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryMidgroundDecorations;
		extern std::vector<::Components::Decoration>& foregroundDecorations;
		extern std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryForegroundDecorations;
		extern std::unordered_map<::ComponentId, ::Components::Missile>& missiles;
		extern std::unordered_map<::ComponentId, ::Components::CollisionHandler>& beginCollisionHandlers;
		extern std::unordered_map<::ComponentId, ::Components::CollisionHandler>& endCollisionHandlers;
	}

	namespace Systems
	{
		void Initialize();

		::Systems::StateController& AccessStateController();
		::Systems::Physics& AccessPhysics();
		::Systems::Textures& AccessTextures();
		::Systems::Players& AccessPlayers();
		::Systems::Walls& AccessWalls();
		::Systems::Camera& AccessCamera();
		::Systems::Decorations& AccessDecorations();
		::Systems::Temporaries& AccessTemporaries();
		::Systems::Cleaner& AccessCleaner();
	}
}
