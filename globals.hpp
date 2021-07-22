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
	struct Camera;
	struct Decoration;
	struct GraphicsSettings;
	struct Missile;
	struct CollisionHandler;
	struct Shockwave;
	struct Light;
	struct Framebuffers;
	struct Functor;
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
	class DeferredActions;
	class RenderingController;
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
		extern ::Components::Framebuffers& framebuffers;

		extern std::vector<::Components::TextureDef>& texturesDef;
		extern std::vector<::Components::Texture>& textures;
		extern std::vector<::Components::Player>& players;
		extern std::vector<::Components::Wall>& staticWalls;
		extern std::vector<::Components::Wall>& dynamicWalls;
		extern std::vector<::Components::Grapple>& grapples;
		extern std::vector<::Components::Decoration>& backgroundDecorations;
		extern std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryBackgroundDecorations;
		extern std::vector<::Components::Decoration>& farMidgroundDecorations;
		extern std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryFarMidgroundDecorations;
		extern std::vector<::Components::Decoration>& midgroundDecorations;
		extern std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryMidgroundDecorations;
		extern std::vector<::Components::Decoration>& nearMidgroundDecorations;
		extern std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryNearMidgroundDecorations;
		extern std::vector<::Components::Decoration>& foregroundDecorations;
		extern std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryForegroundDecorations;
		extern std::unordered_map<::ComponentId, ::Components::Missile>& missiles;
		extern std::unordered_map<::ComponentId, ::Components::CollisionHandler>& beginCollisionHandlers;
		extern std::unordered_map<::ComponentId, ::Components::CollisionHandler>& endCollisionHandlers;
		extern std::unordered_map<::ComponentId, ::Components::Shockwave>& shockwaves;
		extern std::unordered_map<::ComponentId, ::Components::Light>& lights;
		extern std::unordered_map<::ComponentId, ::Components::Functor>& frameSetups;
		extern std::unordered_map<::ComponentId, ::Components::Functor>& frameTeardowns;

		void Reset();
	}

	namespace Systems
	{
		void Initialize();

		::Systems::StateController& StateController();
		::Systems::Physics& Physics();
		::Systems::Textures& Textures();
		::Systems::Players& Players();
		::Systems::Walls& Walls();
		::Systems::Camera& Camera();
		::Systems::Decorations& Decorations();
		::Systems::Temporaries& Temporaries();
		::Systems::Cleaner& Cleaner();
		::Systems::DeferredActions& DeferredActions();
		::Systems::RenderingController& RenderingController();
	}
}
