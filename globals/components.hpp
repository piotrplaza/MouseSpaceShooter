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
	struct Music;
	struct SoundBuffer;
	struct Sound;
	struct RenderingSetup;
	struct Plane;
	struct StaticWall;
	struct DynamicWall;
	struct Grapple;
	struct StaticPolyline;
	struct DynamicPolyline;
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
		StaticComponents<Components::Music>& musics();
		StaticComponents<Components::SoundBuffer>& soundsBuffers();
		DynamicComponents<Components::Sound>& sounds();
		StaticComponents<Components::RenderingSetup>& renderingSetups();
		DynamicComponents<Components::Plane>& planes();
		StaticComponents<Components::StaticWall>& staticWalls();
		DynamicComponents<Components::DynamicWall>& dynamicWalls();
		DynamicComponents<Components::Grapple>& grapples();
		StaticComponents<Components::StaticPolyline>& staticPolylines();
		DynamicComponents<Components::DynamicPolyline>& dynamicPolylines();
		StaticComponents<Components::Decoration>& staticDecorations();
		DynamicComponents<Components::Decoration>& dynamicDecorations();
		DynamicComponents<Components::Missile>& missiles();
		DynamicComponents<Components::CollisionFilter>& collisionFilters();
		DynamicComponents<Components::CollisionHandler>& beginCollisionHandlers();
		DynamicComponents<Components::CollisionHandler>& endCollisionHandlers();
		DynamicComponents<Components::Shockwave>& shockwaves();
		DynamicComponents<Components::Light>& lights();
		DynamicComponents<Components::Functor>& stepSetups();
		DynamicComponents<Components::Functor>& stepTeardowns();
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
		std::unique_ptr<StaticComponents<Components::Music>> musics_ = std::make_unique<StaticComponents<Components::Music>>();
		std::unique_ptr<StaticComponents<Components::SoundBuffer>> soundsBuffers_ = std::make_unique<StaticComponents<Components::SoundBuffer>>();
		std::unique_ptr<DynamicComponents<Components::Sound>> sounds_ = std::make_unique<DynamicComponents<Components::Sound>>();
		std::unique_ptr<StaticComponents<Components::RenderingSetup>> renderingSetups_ = std::make_unique<StaticComponents<Components::RenderingSetup>>();
		std::unique_ptr<DynamicComponents<Components::Plane>> planes_ = std::make_unique<DynamicComponents<Components::Plane>>();
		std::unique_ptr<StaticComponents<Components::StaticWall>> staticWalls_ = std::make_unique<StaticComponents<Components::StaticWall>>();
		std::unique_ptr<DynamicComponents<Components::DynamicWall>> dynamicWalls_ = std::make_unique<DynamicComponents<Components::DynamicWall>>();
		std::unique_ptr<DynamicComponents<Components::Grapple>> grapples_ = std::make_unique<DynamicComponents<Components::Grapple>>();
		std::unique_ptr<StaticComponents<Components::StaticPolyline>> staticPolylines_ = std::make_unique<StaticComponents<Components::StaticPolyline>>();
		std::unique_ptr<DynamicComponents<Components::DynamicPolyline>> dynamicPolylines_ = std::make_unique<DynamicComponents<Components::DynamicPolyline>>();
		std::unique_ptr<StaticComponents<Components::Decoration>> staticDecorations_ = std::make_unique<StaticComponents<Components::Decoration>>();
		std::unique_ptr<DynamicComponents<Components::Decoration>> dynamicDecorations_ = std::make_unique<DynamicComponents<Components::Decoration>>();
		std::unique_ptr<DynamicComponents<Components::Missile>> missiles_ = std::make_unique<DynamicComponents<Components::Missile>>();
		std::unique_ptr<DynamicComponents<Components::CollisionFilter>> collisionFilters_ = std::make_unique<DynamicComponents<Components::CollisionFilter>>();
		std::unique_ptr<DynamicComponents<Components::CollisionHandler>> beginCollisionHandlers_ = std::make_unique<DynamicComponents<Components::CollisionHandler>>();
		std::unique_ptr<DynamicComponents<Components::CollisionHandler>> endCollisionHandlers_ = std::make_unique<DynamicComponents<Components::CollisionHandler>>();
		std::unique_ptr<DynamicComponents<Components::Shockwave>> shockwaves_ = std::make_unique<DynamicComponents<Components::Shockwave>>();
		std::unique_ptr<DynamicComponents<Components::Light>> lights_ = std::make_unique<DynamicComponents<Components::Light>>();
		std::unique_ptr<DynamicComponents<Components::Functor>> stepSetups_ = std::make_unique<DynamicComponents<Components::Functor>>();
		std::unique_ptr<DynamicComponents<Components::Functor>> stepTeardowns_ = std::make_unique<DynamicComponents<Components::Functor>>();
		std::unique_ptr<DynamicOrderedComponents<Components::DeferredAction>> deferredActions_ = std::make_unique<DynamicOrderedComponents<Components::DeferredAction>>();
	};

	void InitializeComponents();
	void MarkDynamicComponentsAsDirty();

	ComponentsHolder& Components();
}
