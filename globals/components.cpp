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
#include <components/renderingSetup.hpp>
#include <components/plane.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
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

namespace Globals
{
	static std::unique_ptr<::ComponentIdGenerator> componentIdGenerator;
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


	std::deque<Components::Texture>& ComponentsHolder::textures()
	{
		return *textures_;
	}

	std::deque<Components::AnimatedTexture>& ComponentsHolder::animatedTextures()
	{
		return *animatedTextures_;
	}

	std::deque<Components::BlendingTexture>& ComponentsHolder::blendingTextures()
	{
		return *blendingTextures_;
	}

	std::deque<Components::RenderingSetup>& ComponentsHolder::renderingSetups()
	{
		return *renderingSetups_;
	}

	std::unordered_map<ComponentId, Components::Plane>& ComponentsHolder::planes()
	{
		return planes_;
	}

	std::deque<Components::Wall>& ComponentsHolder::walls()
	{
		return *structures_;
	}

	std::unordered_map<ComponentId, Components::Wall>& ComponentsHolder::dynamicWalls()
	{
		return dynamicWalls_;
	}

	std::unordered_map<ComponentId, Components::Grapple>& ComponentsHolder::grapples()
	{
		return grapples_;
	}

	std::deque<Components::Decoration>& ComponentsHolder::decorations()
	{
		return *decorations_;
	}

	std::unordered_map<ComponentId, ::Components::Decoration>& ComponentsHolder::dynamicDecorations()
	{
		return dynamicDecorations_;
	}

	std::unordered_map<::ComponentId, Components::Missile>& ComponentsHolder::missiles()
	{
		return missiles_;
	}

	std::unordered_map<::ComponentId, Components::CollisionFilter>& ComponentsHolder::collisionFilters()
	{
		return collisionFilters_;
	}

	std::unordered_map<::ComponentId, Components::CollisionHandler>& ComponentsHolder::beginCollisionHandlers()
	{
		return beginCollisionHandlers_;
	}

	std::unordered_map<::ComponentId, Components::CollisionHandler>& ComponentsHolder::endCollisionHandlers()
	{
		return endCollisionHandlers_;
	}

	std::unordered_map<::ComponentId, Components::Shockwave>& ComponentsHolder::shockwaves()
	{
		return shockwaves_;
	}

	std::unordered_map<::ComponentId, Components::Light>& ComponentsHolder::lights()
	{
		return lights_;
	}

	std::unordered_map<::ComponentId, Components::Functor>& ComponentsHolder::frameSetups()
	{
		return frameSetups_;
	}

	std::unordered_map<::ComponentId, Components::Functor>& ComponentsHolder::frameTeardowns()
	{
		return frameTeardowns_;
	}

	std::list<Components::DeferredAction>& ComponentsHolder::deferredActions()
	{
		return deferredActions_;
	}

	void InitializeComponents()
	{
		componentIdGenerator = std::make_unique<::ComponentIdGenerator>();
		componentsHolder = std::make_unique<ComponentsHolder>();
	}

	::ComponentIdGenerator& ComponentIdGenerator()
	{
		return *componentIdGenerator;
	}

	class ComponentsHolder& Components()
	{
		return *componentsHolder;
	}
}
