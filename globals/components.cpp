#include "components.hpp"

#include <components/renderingBuffers.hpp>
#include <components/keyboard.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>
#include <components/systemInfo.hpp>
#include <components/mvp.hpp>
#include <components/physics.hpp>
#include <components/texture.hpp>
#include <components/animatedTexture.hpp>
#include <components/blendingTexture.hpp>
#include <components/audioListener.hpp>
#include <components/music.hpp>
#include <components/soundBuffer.hpp>
#include <components/sound.hpp>
#include <components/actor.hpp>
#include <components/plane.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/polyline.hpp>
#include <components/camera2D.hpp>
#include <components/camera3D.hpp>
#include <components/decoration.hpp>
#include <components/particles.hpp>
#include <components/graphicsSettings.hpp>
#include <components/missile.hpp>
#include <components/collisionFilter.hpp>
#include <components/collisionHandler.hpp>
#include <components/shockwave.hpp>
#include <components/light2D.hpp>
#include <components/light3D.hpp>
#include <components/framebuffers.hpp>
#include <components/functor.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/deferredAction.hpp>
#include <components/pauseHandler.hpp>

#include "componentIdGenerator.hpp"

namespace Globals
{
	static std::unique_ptr<ComponentsHolder> componentsHolder;

	Components::Defaults& ComponentsHolder::defaults()
	{
		return *defaults_;
	}

	Components::RenderingBuffers& ComponentsHolder::renderingBuffers()
	{
		return *renderingBuffers_;
	}

	Components::Keyboard& ComponentsHolder::keyboard()
	{
		return *keyboardState_;
	}

	Components::Mouse& ComponentsHolder::mouse()
	{
		return *mouseState_;
	}

	std::array<Components::Gamepad, 4>& ComponentsHolder::gamepads()
	{
		return *gamepads_;
	}

	Components::SystemInfo& ComponentsHolder::systemInfo()
	{
		return *systemInfo_;
	}

	Components::MVP& ComponentsHolder::mvp2D()
	{
		return *mvp2D_;
	}

	Components::MVP& ComponentsHolder::mvp3D()
	{
		return *mvp3D_;
	}

	Components::Physics& ComponentsHolder::physics()
	{
		return *physics_;
	}

	Components::Camera2D& ComponentsHolder::camera2D()
	{
		return *camera_;
	}

	Components::Camera3D& ComponentsHolder::camera3D()
	{
		return *camera3D_;
	}

	Components::GraphicsSettings& ComponentsHolder::graphicsSettings()
	{
		return *graphicsSettings_;
	}

	Components::Framebuffers& ComponentsHolder::framebuffers()
	{
		return *framebuffers_;
	}

	Components::MainFramebufferRenderer& ComponentsHolder::mainFramebufferRenderer()
	{
		return *mainFramebufferRenderer_;
	}

	Components::AudioListener& ComponentsHolder::audioListener()
	{
		return *audioListener_;
	}

	Components::PauseHandler& ComponentsHolder::pauseHandler()
	{
		return *pauseHandler_;
	}

	StaticComponents<Components::Texture>& ComponentsHolder::staticTextures()
	{
		return *staticTextures_;
	}

	DynamicComponents<Components::Texture>& ComponentsHolder::dynamicTextures()
	{
		return *dynamicTextures_;
	}

	StaticComponents<Components::AnimatedTexture>& ComponentsHolder::staticAnimatedTextures()
	{
		return *staticAnimatedTextures_;
	}

	DynamicComponents<Components::AnimatedTexture>& ComponentsHolder::dynamicAnimatedTextures()
	{
		return *dynamicAnimatedTextures_;
	}

	StaticComponents<Components::BlendingTexture>& ComponentsHolder::staticBlendingTextures()
	{
		return *staticBlendingTextures_;
	}

	DynamicComponents<Components::BlendingTexture>& ComponentsHolder::dynamicBlendingTextures()
	{
		return *dynamicBlendingTextures_;
	}

	StaticComponents<Components::Music>& ComponentsHolder::musics()
	{
		return *musics_;
	}

	StaticComponents<Components::SoundBuffer>& ComponentsHolder::soundsBuffers()
	{
		return *soundsBuffers_;
	}

	DynamicComponents<Components::Sound>& ComponentsHolder::sounds()
	{
		return *sounds_;
	}

	DynamicComponents<Components::Actor>& ComponentsHolder::actors()
	{
		return *actors_;
	}

	DynamicComponents<Components::Plane>& ComponentsHolder::planes()
	{
		return *planes_;
	}

	StaticComponents<Components::Wall>& ComponentsHolder::staticWalls()
	{
		return *staticWalls_;
	}

	DynamicComponents<Components::Wall>& ComponentsHolder::dynamicWalls()
	{
		return *dynamicWalls_;
	}

	DynamicComponents<Components::Grapple>& ComponentsHolder::grapples()
	{
		return *grapples_;
	}

	StaticComponents<Components::Polyline>& ComponentsHolder::staticPolylines()
	{
		return *staticPolylines_;
	}
	
	DynamicComponents<Components::Polyline>& ComponentsHolder::dynamicPolylines()
	{
		return *dynamicPolylines_;
	}

	StaticComponents<Components::Decoration>& ComponentsHolder::staticDecorations()
	{
		return *staticDecorations_;
	}

	DynamicComponents<Components::Decoration>& ComponentsHolder::dynamicDecorations()
	{
		return *dynamicDecorations_;
	}

	StaticComponents<Components::Particles>& ComponentsHolder::staticParticles()
	{
		return *staticParticles_;
	}

	DynamicComponents<Components::Particles>& ComponentsHolder::dynamicParticles()
	{
		return *dynamicParticles_;
	}

	DynamicComponents<Components::Missile>& ComponentsHolder::missiles()
	{
		return *missiles_;
	}

	DynamicComponents<Components::CollisionFilter>& ComponentsHolder::collisionFilters()
	{
		return *collisionFilters_;
	}

	DynamicComponents<Components::CollisionHandler>& ComponentsHolder::beginCollisionHandlers()
	{
		return *beginCollisionHandlers_;
	}

	DynamicComponents<Components::CollisionHandler>& ComponentsHolder::endCollisionHandlers()
	{
		return *endCollisionHandlers_;
	}

	DynamicComponents<Components::Shockwave>& ComponentsHolder::shockwaves()
	{
		return *shockwaves_;
	}

	DynamicComponents<Components::Light2D>& ComponentsHolder::lights2D()
	{
		return *lights2D_;
	}

	DynamicComponents<Components::Light3D>& ComponentsHolder::lights3D()
	{
		return *lights3D_;
	}

	DynamicOrderedComponents<Components::Functor>& ComponentsHolder::postInits()
	{
		return *postInits_;
	}

	DynamicOrderedComponents<Components::Functor>& ComponentsHolder::stepSetups()
	{
		return *stepSetups_;
	}

	DynamicOrderedComponents<Components::Functor>& ComponentsHolder::stepTeardowns()
	{
		return *stepTeardowns_;
	}

	DynamicOrderedComponents<Components::Functor>& ComponentsHolder::renderSetups()
	{
		return *renderSetups_;
	}

	DynamicOrderedComponents<Components::Functor>& ComponentsHolder::renderTeardowns()
	{
		return *renderTeardown_;
	}

	DynamicOrderedComponents<Components::DeferredAction>& ComponentsHolder::deferredActions()
	{
		return *deferredActions_;
	}

	void InitializeComponents()
	{
		InitializeComponentIdGenerator();
		componentsHolder = std::make_unique<ComponentsHolder>();
	}

	void DestroyComponents()
	{
		componentsHolder.reset();
	}

	void MarkDynamicComponentsAsDirty()
	{
		DynamicComponentsBase::markAllAsDirty();
	}

	ComponentsHolder& Components()
	{
		return *componentsHolder;
	}
}
