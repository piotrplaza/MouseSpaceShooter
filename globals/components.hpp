#pragma once

#include <components/_componentId.hpp>

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

		std::deque<Components::Texture>& textures();
		std::deque<Components::AnimatedTexture>& animatedTextures();
		std::deque<Components::BlendingTexture>& blendingTextures();
		std::deque<Components::RenderingSetup>& renderingSetups();
		std::unordered_map<ComponentId, Components::Plane>& planes();
		std::deque<Components::Wall>& walls();
		std::unordered_map<ComponentId, Components::Wall>& dynamicWalls();
		std::unordered_map<ComponentId, Components::Grapple>& grapples();
		std::deque<Components::Decoration>& decorations();
		std::unordered_map<ComponentId, Components::Decoration>& dynamicDecorations();
		std::unordered_map<ComponentId, Components::Missile>& missiles();
		std::unordered_map<ComponentId, Components::CollisionFilter>& collisionFilters();
		std::unordered_map<ComponentId, Components::CollisionHandler>& beginCollisionHandlers();
		std::unordered_map<ComponentId, Components::CollisionHandler>& endCollisionHandlers();
		std::unordered_map<ComponentId, Components::Shockwave>& shockwaves();
		std::unordered_map<ComponentId, Components::Light>& lights();
		std::unordered_map<ComponentId, Components::Functor>& frameSetups();
		std::unordered_map<ComponentId, Components::Functor>& frameTeardowns();
		std::list<Components::DeferredAction>& deferredActions();

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

		std::unique_ptr<std::deque<Components::Texture>> textures_ = std::make_unique<std::deque<Components::Texture>>();
		std::unique_ptr<std::deque<Components::AnimatedTexture>> animatedTextures_ = std::make_unique<std::deque<Components::AnimatedTexture>>();
		std::unique_ptr<std::deque<Components::BlendingTexture>> blendingTextures_ = std::make_unique<std::deque<Components::BlendingTexture>>();
		std::unique_ptr<std::deque<Components::RenderingSetup>> renderingSetups_ = std::make_unique<std::deque<Components::RenderingSetup>>();
		std::unordered_map<ComponentId, Components::Plane> planes_;
		std::unique_ptr<std::deque<Components::Wall>> structures_ = std::make_unique<std::deque<Components::Wall>>();
		std::unordered_map<ComponentId, Components::Wall> dynamicWalls_;
		std::unordered_map<ComponentId, Components::Grapple> grapples_;
		std::unique_ptr<std::deque<Components::Decoration>> decorations_ = std::make_unique<std::deque<Components::Decoration>>();
		std::unordered_map<ComponentId, Components::Decoration> dynamicDecorations_;
		std::unordered_map<ComponentId, Components::Missile> missiles_;
		std::unordered_map<ComponentId, Components::CollisionFilter> collisionFilters_;
		std::unordered_map<ComponentId, Components::CollisionHandler> beginCollisionHandlers_;
		std::unordered_map<ComponentId, Components::CollisionHandler> endCollisionHandlers_;
		std::unordered_map<ComponentId, Components::Shockwave> shockwaves_;
		std::unordered_map<ComponentId, Components::Light> lights_;
		std::unordered_map<ComponentId, Components::Functor> frameSetups_;
		std::unordered_map<ComponentId, Components::Functor> frameTeardowns_;
		std::list<Components::DeferredAction> deferredActions_;
	};

	void InitializeComponents();

	::ComponentIdGenerator& ComponentIdGenerator();
	ComponentsHolder& Components();
}
