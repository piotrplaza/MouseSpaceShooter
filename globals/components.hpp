#pragma once

#include <commonTypes/componentsContainers.hpp>

#include <memory>
#include <deque>
#include <array>
#include <unordered_map>
#include <list>

namespace Components
{
	struct RenderingBuffers;
	struct Keyboard;
	struct Mouse;
	struct Gamepad;
	struct ScreenInfo;
	struct MVP;
	struct Physics;
	struct Texture;
	struct AnimatedTexture;
	struct BlendingTexture;
	struct RenderingSetup;
	struct Plane;
	struct Wall;
	struct Grapple;
	struct Polyline;
	struct Camera;
	struct Decoration;
	struct GraphicsSettings;
	struct Missile;
	struct CollisionFilter;
	struct CollisionHandler;
	struct Shockwave;
	struct Light;
	struct Framebuffers;
	struct Functor;
	struct MainFramebufferRenderer;
	struct DeferredAction;
}

namespace Globals
{
	class ComponentsHolder
	{
	public:
		Components::RenderingBuffers& renderingBuffers();
		Components::Keyboard& keyboard();
		Components::Mouse& mouse();
		std::array<Components::Gamepad, 4>& gamepads();
		Components::ScreenInfo& screenInfo();
		Components::MVP& mvp();
		Components::Physics& physics();
		Components::Camera& camera();
		Components::GraphicsSettings& graphicsSettings();
		Components::Framebuffers& framebuffers();
		Components::MainFramebufferRenderer& mainFramebufferRenderer();

		StaticComponents<Components::Texture>& textures();
		StaticComponents<Components::AnimatedTexture>& animatedTextures();
		StaticComponents<Components::BlendingTexture>& blendingTextures();
		StaticComponents<Components::RenderingSetup>& renderingSetups();
		DynamicComponents<Components::Plane>& planes();
		StaticComponents<Components::Wall>& walls();
		DynamicComponents<Components::Wall>& dynamicWalls();
		DynamicComponents<Components::Grapple>& grapples();
		StaticComponents<Components::Polyline>& polylines();
		StaticComponents<Components::Decoration>& decorations();
		DynamicComponents<Components::Decoration>& dynamicDecorations();
		DynamicComponents<Components::Missile>& missiles();
		DynamicComponents<Components::CollisionFilter>& collisionFilters();
		DynamicComponents<Components::CollisionHandler>& beginCollisionHandlers();
		DynamicComponents<Components::CollisionHandler>& endCollisionHandlers();
		DynamicComponents<Components::Shockwave>& shockwaves();
		DynamicComponents<Components::Light>& lights();
		DynamicComponents<Components::Functor>& frameSetups();
		DynamicComponents<Components::Functor>& frameTeardowns();
		DynamicOrderedComponents<Components::DeferredAction>& deferredActions();

	private:
		std::unique_ptr<Components::RenderingBuffers> renderingBuffers_ = std::make_unique<Components::RenderingBuffers>();
		std::unique_ptr<Components::Keyboard> keyboardState_ = std::make_unique<Components::Keyboard>();
		std::unique_ptr<Components::Mouse> mouseState_ = std::make_unique<Components::Mouse>();
		std::unique_ptr<std::array<Components::Gamepad, 4>> gamepads_ = std::make_unique< std::array<Components::Gamepad, 4>>();
		std::unique_ptr<Components::ScreenInfo> screenInfo_ = std::make_unique<Components::ScreenInfo>();
		std::unique_ptr<Components::MVP> mvp_ = std::make_unique<Components::MVP>();
		std::unique_ptr<Components::Physics> physics_ = std::make_unique<Components::Physics>();
		std::unique_ptr<Components::Camera> camera_ = std::make_unique<Components::Camera>();
		std::unique_ptr<Components::GraphicsSettings> graphicsSettings_ = std::make_unique<Components::GraphicsSettings>();
		std::unique_ptr<Components::Framebuffers> framebuffers_ = std::make_unique<Components::Framebuffers>();
		std::unique_ptr<Components::MainFramebufferRenderer> mainFramebufferRenderer_ = std::make_unique<Components::MainFramebufferRenderer>();

		std::unique_ptr<StaticComponents<Components::Texture>> textures_ = std::make_unique<StaticComponents<Components::Texture>>();
		std::unique_ptr<StaticComponents<Components::AnimatedTexture>> animatedTextures_ = std::make_unique<StaticComponents<Components::AnimatedTexture>>();
		std::unique_ptr<StaticComponents<Components::BlendingTexture>> blendingTextures_ = std::make_unique<StaticComponents<Components::BlendingTexture>>();
		std::unique_ptr<StaticComponents<Components::RenderingSetup>> renderingSetups_ = std::make_unique<StaticComponents<Components::RenderingSetup>>();
		DynamicComponents<Components::Plane> planes_;
		std::unique_ptr<StaticComponents<Components::Wall>> walls_ = std::make_unique<StaticComponents<Components::Wall>>();
		DynamicComponents<Components::Wall> dynamicWalls_;
		DynamicComponents<Components::Grapple> grapples_;
		std::unique_ptr<StaticComponents<Components::Polyline>> polylines_ = std::make_unique<StaticComponents<Components::Polyline>>();
		std::unique_ptr<StaticComponents<Components::Decoration>> decorations_ = std::make_unique<StaticComponents<Components::Decoration>>();
		DynamicComponents<Components::Decoration> dynamicDecorations_;
		DynamicComponents<Components::Missile> missiles_;
		DynamicComponents<Components::CollisionFilter> collisionFilters_;
		DynamicComponents<Components::CollisionHandler> beginCollisionHandlers_;
		DynamicComponents<Components::CollisionHandler> endCollisionHandlers_;
		DynamicComponents<Components::Shockwave> shockwaves_;
		DynamicComponents<Components::Light> lights_;
		DynamicComponents<Components::Functor> frameSetups_;
		DynamicComponents<Components::Functor> frameTeardowns_;
		DynamicOrderedComponents<Components::DeferredAction> deferredActions_;
	};

	void InitializeComponents();

	ComponentsHolder& Components();
}
