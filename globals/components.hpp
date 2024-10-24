#pragma once

#include <commonTypes/componentsContainers.hpp>

#include <memory>
#include <deque>
#include <array>
#include <unordered_map>
#include <list>

namespace Components
{
	struct Defaults;
	struct RenderingBuffers;
	struct Keyboard;
	struct Mouse;
	struct Gamepad;
	struct SystemInfo;
	struct MVP;
	struct Physics;
	struct Texture;
	struct AnimatedTexture;
	struct BlendingTexture;
	struct AudioListener;
	struct Music;
	struct SoundBuffer;
	struct Sound;
	struct Actor;
	struct Plane;
	struct Wall;
	struct Grapple;
	struct Polyline;
	struct Camera2D;
	struct Camera3D;
	struct Decoration;
	struct Particles;
	struct GraphicsSettings;
	struct Missile;
	struct CollisionFilter;
	struct CollisionHandler;
	struct Shockwave;
	struct Light2D;
	struct Light3D;
	struct Framebuffers;
	struct Functor;
	struct MainFramebufferRenderer;
	struct DeferredAction;
	struct PauseHandler;
}

namespace Globals
{
	class ComponentsHolder
	{
	public:
		Components::Defaults& defaults();
		Components::RenderingBuffers& renderingBuffers();
		Components::Keyboard& keyboard();
		Components::Mouse& mouse();
		std::array<Components::Gamepad, 4>& gamepads();
		Components::SystemInfo& systemInfo();
		Components::MVP& mvp2D();
		Components::MVP& mvp3D();
		Components::Physics& physics();
		Components::Camera2D& camera2D();
		Components::Camera3D& camera3D();
		Components::GraphicsSettings& graphicsSettings();
		Components::Framebuffers& framebuffers();
		Components::MainFramebufferRenderer& mainFramebufferRenderer();
		Components::AudioListener& audioListener();
		Components::PauseHandler& pauseHandler();

		StaticComponents<Components::Texture>& staticTextures();
		DynamicComponents<Components::Texture>& dynamicTextures();
		StaticComponents<Components::AnimatedTexture>& staticAnimatedTextures();
		DynamicComponents<Components::AnimatedTexture>& dynamicAnimatedTextures();
		StaticComponents<Components::BlendingTexture>& staticBlendingTextures();
		DynamicComponents<Components::BlendingTexture>& dynamicBlendingTextures();
		StaticComponents<Components::Music>& musics();
		StaticComponents<Components::SoundBuffer>& soundsBuffers();
		DynamicComponents<Components::Sound>& sounds();
		DynamicComponents<Components::Actor>& actors();
		DynamicComponents<Components::Plane>& planes();
		StaticComponents<Components::Wall>& staticWalls();
		DynamicComponents<Components::Wall>& dynamicWalls();
		DynamicComponents<Components::Grapple>& grapples();
		StaticComponents<Components::Polyline>& staticPolylines();
		DynamicComponents<Components::Polyline>& dynamicPolylines();
		StaticComponents<Components::Decoration>& staticDecorations();
		DynamicComponents<Components::Decoration>& dynamicDecorations();
		StaticComponents<Components::Particles>& staticParticles();
		DynamicComponents<Components::Particles>& dynamicParticles();
		DynamicComponents<Components::Missile>& missiles();
		DynamicComponents<Components::CollisionFilter>& collisionFilters();
		DynamicComponents<Components::CollisionHandler>& beginCollisionHandlers();
		DynamicComponents<Components::CollisionHandler>& endCollisionHandlers();
		DynamicComponents<Components::Shockwave>& shockwaves();
		DynamicComponents<Components::Light2D>& lights2D();
		DynamicComponents<Components::Light3D>& lights3D();
		DynamicOrderedComponents<Components::Functor>& postInits();
		DynamicOrderedComponents<Components::Functor>& stepSetups();
		DynamicOrderedComponents<Components::Functor>& stepTeardowns();
		DynamicOrderedComponents<Components::Functor>& renderSetups();
		DynamicOrderedComponents<Components::Functor>& renderTeardowns();
		DynamicOrderedComponents<Components::DeferredAction>& deferredActions();

	private:
		std::unique_ptr<Components::Defaults> defaults_ = std::make_unique<Components::Defaults>();
		std::unique_ptr<Components::RenderingBuffers> renderingBuffers_ = std::make_unique<Components::RenderingBuffers>();
		std::unique_ptr<Components::Keyboard> keyboardState_ = std::make_unique<Components::Keyboard>();
		std::unique_ptr<Components::Mouse> mouseState_ = std::make_unique<Components::Mouse>();
		std::unique_ptr<std::array<Components::Gamepad, 4>> gamepads_ = std::make_unique< std::array<Components::Gamepad, 4>>();
		std::unique_ptr<Components::SystemInfo> systemInfo_ = std::make_unique<Components::SystemInfo>();
		std::unique_ptr<Components::MVP> mvp2D_ = std::make_unique<Components::MVP>();
		std::unique_ptr<Components::MVP> mvp3D_ = std::make_unique<Components::MVP>();
		std::unique_ptr<Components::Physics> physics_ = std::make_unique<Components::Physics>();
		std::unique_ptr<Components::Camera2D> camera_ = std::make_unique<Components::Camera2D>();
		std::unique_ptr<Components::Camera3D> camera3D_ = std::make_unique<Components::Camera3D>();
		std::unique_ptr<Components::GraphicsSettings> graphicsSettings_ = std::make_unique<Components::GraphicsSettings>();
		std::unique_ptr<Components::Framebuffers> framebuffers_ = std::make_unique<Components::Framebuffers>();
		std::unique_ptr<Components::MainFramebufferRenderer> mainFramebufferRenderer_ = std::make_unique<Components::MainFramebufferRenderer>();
		std::unique_ptr<Components::AudioListener> audioListener_ = std::make_unique<Components::AudioListener>();
		std::unique_ptr<Components::PauseHandler> pauseHandler_ = std::make_unique<Components::PauseHandler>();

		std::unique_ptr<StaticComponents<Components::Texture>> staticTextures_ = std::make_unique<StaticComponents<Components::Texture>>();
		std::unique_ptr<DynamicComponents<Components::Texture>> dynamicTextures_ = std::make_unique<DynamicComponents<Components::Texture>>();
		std::unique_ptr<StaticComponents<Components::AnimatedTexture>> staticAnimatedTextures_ = std::make_unique<StaticComponents<Components::AnimatedTexture>>();
		std::unique_ptr<DynamicComponents<Components::AnimatedTexture>> dynamicAnimatedTextures_ = std::make_unique<DynamicComponents<Components::AnimatedTexture>>();
		std::unique_ptr<StaticComponents<Components::BlendingTexture>> staticBlendingTextures_ = std::make_unique<StaticComponents<Components::BlendingTexture>>();
		std::unique_ptr<DynamicComponents<Components::BlendingTexture>> dynamicBlendingTextures_ = std::make_unique<DynamicComponents<Components::BlendingTexture>>();
		std::unique_ptr<StaticComponents<Components::Music>> musics_ = std::make_unique<StaticComponents<Components::Music>>();
		std::unique_ptr<StaticComponents<Components::SoundBuffer>> soundsBuffers_ = std::make_unique<StaticComponents<Components::SoundBuffer>>();
		std::unique_ptr<DynamicComponents<Components::Sound>> sounds_ = std::make_unique<DynamicComponents<Components::Sound>>();
		std::unique_ptr<DynamicComponents<Components::Actor>> actors_ = std::make_unique<DynamicComponents<Components::Actor>>();
		std::unique_ptr<DynamicComponents<Components::Plane>> planes_ = std::make_unique<DynamicComponents<Components::Plane>>();
		std::unique_ptr<StaticComponents<Components::Wall>> staticWalls_ = std::make_unique<StaticComponents<Components::Wall>>();
		std::unique_ptr<DynamicComponents<Components::Wall>> dynamicWalls_ = std::make_unique<DynamicComponents<Components::Wall>>();
		std::unique_ptr<DynamicComponents<Components::Grapple>> grapples_ = std::make_unique<DynamicComponents<Components::Grapple>>();
		std::unique_ptr<StaticComponents<Components::Polyline>> staticPolylines_ = std::make_unique<StaticComponents<Components::Polyline>>();
		std::unique_ptr<DynamicComponents<Components::Polyline>> dynamicPolylines_ = std::make_unique<DynamicComponents<Components::Polyline>>();
		std::unique_ptr<StaticComponents<Components::Decoration>> staticDecorations_ = std::make_unique<StaticComponents<Components::Decoration>>();
		std::unique_ptr<DynamicComponents<Components::Decoration>> dynamicDecorations_ = std::make_unique<DynamicComponents<Components::Decoration>>();
		std::unique_ptr<StaticComponents<Components::Particles>> staticParticles_ = std::make_unique<StaticComponents<Components::Particles>>();
		std::unique_ptr<DynamicComponents<Components::Particles>> dynamicParticles_ = std::make_unique<DynamicComponents<Components::Particles>>();
		std::unique_ptr<DynamicComponents<Components::Missile>> missiles_ = std::make_unique<DynamicComponents<Components::Missile>>();
		std::unique_ptr<DynamicComponents<Components::CollisionFilter>> collisionFilters_ = std::make_unique<DynamicComponents<Components::CollisionFilter>>();
		std::unique_ptr<DynamicComponents<Components::CollisionHandler>> beginCollisionHandlers_ = std::make_unique<DynamicComponents<Components::CollisionHandler>>();
		std::unique_ptr<DynamicComponents<Components::CollisionHandler>> endCollisionHandlers_ = std::make_unique<DynamicComponents<Components::CollisionHandler>>();
		std::unique_ptr<DynamicComponents<Components::Shockwave>> shockwaves_ = std::make_unique<DynamicComponents<Components::Shockwave>>();
		std::unique_ptr<DynamicComponents<Components::Light2D>> lights2D_ = std::make_unique<DynamicComponents<Components::Light2D>>();
		std::unique_ptr<DynamicComponents<Components::Light3D>> lights3D_ = std::make_unique<DynamicComponents<Components::Light3D>>();
		std::unique_ptr<DynamicOrderedComponents<Components::Functor>> postInits_ = std::make_unique<DynamicOrderedComponents<Components::Functor>>();
		std::unique_ptr<DynamicOrderedComponents<Components::Functor>> stepSetups_ = std::make_unique<DynamicOrderedComponents<Components::Functor>>();
		std::unique_ptr<DynamicOrderedComponents<Components::Functor>> stepTeardowns_ = std::make_unique<DynamicOrderedComponents<Components::Functor>>();
		std::unique_ptr<DynamicOrderedComponents<Components::Functor>> renderSetups_ = std::make_unique<DynamicOrderedComponents<Components::Functor>>();
		std::unique_ptr<DynamicOrderedComponents<Components::Functor>> renderTeardown_ = std::make_unique<DynamicOrderedComponents<Components::Functor>>();
		std::unique_ptr<DynamicOrderedComponents<Components::DeferredAction>> deferredActions_ = std::make_unique<DynamicOrderedComponents<Components::DeferredAction>>();
	};

	void InitializeComponents();
	void DestroyComponents();
	void MarkDynamicComponentsAsDirty();

	ComponentsHolder& Components();
}
