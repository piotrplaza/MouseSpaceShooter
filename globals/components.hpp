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
	struct AudioListener;
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
	struct Camera2D;
	struct Camera3D;
	struct Decoration;
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
		Components::MVP& mvp2D();
		Components::MVP& mvp3D();
		Components::Physics& physics();
		Components::Camera2D& camera2D();
		Components::Camera3D& camera3D();
		Components::GraphicsSettings& graphicsSettings();
		Components::Framebuffers& framebuffers();
		Components::MainFramebufferRenderer& mainFramebufferRenderer();
		Components::AudioListener& audioListener();

		StaticComponents<Components::Texture>& staticTextures();
		DynamicComponents<Components::Texture>& dynamicTextures();
		StaticComponents<Components::AnimatedTexture>& staticAnimatedTextures();
		DynamicComponents<Components::AnimatedTexture>& dynamicAnimatedTextures();
		StaticComponents<Components::BlendingTexture>& staticBlendingTextures();
		DynamicComponents<Components::BlendingTexture>& dynamicBlendingTextures();
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
		DynamicComponents<Components::Light2D>& lights2D();
		DynamicComponents<Components::Light3D>& lights3D();
		DynamicOrderedComponents<Components::Functor>& postInits();
		DynamicOrderedComponents<Components::Functor>& stepSetups();
		DynamicOrderedComponents<Components::Functor>& stepTeardowns();
		DynamicOrderedComponents<Components::Functor>& renderSetups();
		DynamicOrderedComponents<Components::Functor>& renderTeardowns();
		DynamicOrderedComponents<Components::DeferredAction>& deferredActions();

	private:
		std::unique_ptr<Components::RenderingBuffers> renderingBuffers_ = std::make_unique<Components::RenderingBuffers>();
		std::unique_ptr<Components::Keyboard> keyboardState_ = std::make_unique<Components::Keyboard>();
		std::unique_ptr<Components::Mouse> mouseState_ = std::make_unique<Components::Mouse>();
		std::unique_ptr<std::array<Components::Gamepad, 4>> gamepads_ = std::make_unique< std::array<Components::Gamepad, 4>>();
		std::unique_ptr<Components::ScreenInfo> screenInfo_ = std::make_unique<Components::ScreenInfo>();
		std::unique_ptr<Components::MVP> mvp2D_ = std::make_unique<Components::MVP>();
		std::unique_ptr<Components::MVP> mvp3D_ = std::make_unique<Components::MVP>();
		std::unique_ptr<Components::Physics> physics_ = std::make_unique<Components::Physics>();
		std::unique_ptr<Components::Camera2D> camera_ = std::make_unique<Components::Camera2D>();
		std::unique_ptr<Components::Camera3D> camera3D_ = std::make_unique<Components::Camera3D>();
		std::unique_ptr<Components::GraphicsSettings> graphicsSettings_ = std::make_unique<Components::GraphicsSettings>();
		std::unique_ptr<Components::Framebuffers> framebuffers_ = std::make_unique<Components::Framebuffers>();
		std::unique_ptr<Components::MainFramebufferRenderer> mainFramebufferRenderer_ = std::make_unique<Components::MainFramebufferRenderer>();
		std::unique_ptr<Components::AudioListener> audioListener_ = std::make_unique<Components::AudioListener>();

		std::unique_ptr<StaticComponents<Components::Texture>> staticTextures_ = std::make_unique<StaticComponents<Components::Texture>>();
		std::unique_ptr<DynamicComponents<Components::Texture>> dynamicTextures_ = std::make_unique<DynamicComponents<Components::Texture>>();
		std::unique_ptr<StaticComponents<Components::AnimatedTexture>> staticAnimatedTextures_ = std::make_unique<StaticComponents<Components::AnimatedTexture>>();
		std::unique_ptr<DynamicComponents<Components::AnimatedTexture>> dynamicAnimatedTextures_ = std::make_unique<DynamicComponents<Components::AnimatedTexture>>();
		std::unique_ptr<StaticComponents<Components::BlendingTexture>> staticBlendingTextures_ = std::make_unique<StaticComponents<Components::BlendingTexture>>();
		std::unique_ptr<DynamicComponents<Components::BlendingTexture>> dynamicBlendingTextures_ = std::make_unique<DynamicComponents<Components::BlendingTexture>>();
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
