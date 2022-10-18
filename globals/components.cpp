#include "components.hpp"

#include <components/renderingBuffers.hpp>
#include <components/keyboard.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>
#include <components/screenInfo.hpp>
#include <components/mvp.hpp>
#include <components/physics.hpp>
#include <components/texture.hpp>
#include <components/animatedTexture.hpp>
#include <components/blendingTexture.hpp>
#include <components/music.hpp>
#include <components/soundBuffer.hpp>
#include <components/sound.hpp>
#include <components/renderingSetup.hpp>
#include <components/plane.hpp>
#include <components/staticWall.hpp>
#include <components/dynamicWall.hpp>
#include <components/grapple.hpp>
#include <components/polyline.hpp>
#include <components/camera.hpp>
#include <components/decoration.hpp>
#include <components/graphicsSettings.hpp>
#include <components/missile.hpp>
#include <components/collisionFilter.hpp>
#include <components/collisionHandler.hpp>
#include <components/shockwave.hpp>
#include <components/light.hpp>
#include <components/framebuffers.hpp>
#include <components/functor.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/deferredAction.hpp>

#include "componentIdGenerator.hpp"

namespace Globals
{
	static std::unique_ptr<ComponentsHolder> componentsHolder;

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

	Components::ScreenInfo& ComponentsHolder::screenInfo()
	{
		return *screenInfo_;
	}

	Components::MVP& ComponentsHolder::mvp()
	{
		return *mvp_;
	}

	Components::Physics& ComponentsHolder::physics()
	{
		return *physics_;
	}

	Components::Camera& ComponentsHolder::camera()
	{
		return *camera_;
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

	StaticComponents<Components::Texture>& ComponentsHolder::textures()
	{
		return *textures_;
	}

	StaticComponents<Components::AnimatedTexture>& ComponentsHolder::animatedTextures()
	{
		return *animatedTextures_;
	}

	StaticComponents<Components::BlendingTexture>& ComponentsHolder::blendingTextures()
	{
		return *blendingTextures_;
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
		return sounds_;
	}

	StaticComponents<Components::RenderingSetup>& ComponentsHolder::renderingSetups()
	{
		return *renderingSetups_;
	}

	DynamicComponents<Components::Plane>& ComponentsHolder::planes()
	{
		return planes_;
	}

	StaticComponents<Components::StaticWall>& ComponentsHolder::staticWalls()
	{
		return *staticWalls_;
	}

	DynamicComponents<Components::DynamicWall>& ComponentsHolder::dynamicWalls()
	{
		return dynamicWalls_;
	}

	DynamicComponents<Components::Grapple>& ComponentsHolder::grapples()
	{
		return grapples_;
	}

	StaticComponents<Components::Polyline>& ComponentsHolder::polylines()
	{
		return *polylines_;
	}

	StaticComponents<Components::Decoration>& ComponentsHolder::staticDecorations()
	{
		return *staticDecorations_;
	}

	DynamicComponents<Components::Decoration>& ComponentsHolder::dynamicDecorations()
	{
		return dynamicDecorations_;
	}

	DynamicComponents<Components::Missile>& ComponentsHolder::missiles()
	{
		return missiles_;
	}

	DynamicComponents<Components::CollisionFilter>& ComponentsHolder::collisionFilters()
	{
		return collisionFilters_;
	}

	DynamicComponents<Components::CollisionHandler>& ComponentsHolder::beginCollisionHandlers()
	{
		return beginCollisionHandlers_;
	}

	DynamicComponents<Components::CollisionHandler>& ComponentsHolder::endCollisionHandlers()
	{
		return endCollisionHandlers_;
	}

	DynamicComponents<Components::Shockwave>& ComponentsHolder::shockwaves()
	{
		return shockwaves_;
	}

	DynamicComponents<Components::Light>& ComponentsHolder::lights()
	{
		return lights_;
	}

	DynamicComponents<Components::Functor>& ComponentsHolder::frameSetups()
	{
		return frameSetups_;
	}

	DynamicComponents<Components::Functor>& ComponentsHolder::frameTeardowns()
	{
		return frameTeardowns_;
	}

	DynamicOrderedComponents<Components::DeferredAction>& ComponentsHolder::deferredActions()
	{
		return deferredActions_;
	}

	void InitializeComponents()
	{
		InitializeComponentIdGenerator();
		componentsHolder = std::make_unique<ComponentsHolder>();
	}

	ComponentsHolder& Components()
	{
		return *componentsHolder;
	}
}
