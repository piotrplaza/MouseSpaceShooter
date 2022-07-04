#include "components.hpp"

#include <components/mouseState.hpp>
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

	Components::MouseState& ComponentsHolder::mouseState()
	{
		return *mouseState_;
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


	std::vector<Components::Texture>& ComponentsHolder::textures()
	{
		return textures_;
	}

	std::vector<Components::AnimatedTexture>& ComponentsHolder::animatedTextures()
	{
		return animatedTextures_;
	}

	std::vector<Components::BlendingTexture>& ComponentsHolder::blendingTextures()
	{
		return blendingTextures_;
	}

	std::vector<Components::RenderingSetup>& ComponentsHolder::renderingSetups()
	{
		return renderingSetups_;
	}

	std::vector<Components::Plane>& ComponentsHolder::planes()
	{
		return planes_;
	}

	std::vector<Components::Wall>& ComponentsHolder::structures()
	{
		return structures_;
	}

	std::unordered_map<ComponentId, Components::Wall>& ComponentsHolder::dynamicWalls()
	{
		return dynamicWalls_;
	}

	std::unordered_map<ComponentId, Components::Grapple>& ComponentsHolder::grapples()
	{
		return grapples_;
	}

	std::vector<Components::Decoration>& ComponentsHolder::backgroundDecorations()
	{
		return backgroundDecorations_;
	}

	std::unordered_map<ComponentId, ::Components::Decoration>& ComponentsHolder::dynamicBackgroundDecorations()
	{
		return dynamicBackgroundDecorations_;
	}

	std::vector<Components::Decoration>& ComponentsHolder::farMidgroundDecorations()
	{
		return farMidgroundDecorations_;
	}

	std::unordered_map<ComponentId, Components::Decoration>& ComponentsHolder::dynamicFarMidgroundDecorations()
	{
		return dynamicFarMidgroundDecorations_;
	}

	std::vector<Components::Decoration>& ComponentsHolder::midgroundDecorations()
	{
		return midgroundDecorations_;
	}

	std::unordered_map<ComponentId, Components::Decoration>& ComponentsHolder::dynamicMidgroundDecorations()
	{
		return dynamicMidgroundDecorations_;
	}

	std::vector<Components::Decoration>& ComponentsHolder::nearMidgroundDecorations()
	{
		return nearMidgroundDecorations_;
	}

	std::unordered_map<ComponentId, Components::Decoration>& ComponentsHolder::dynamicNearMidgroundDecorations()
	{
		return dynamicNearMidgroundDecorations_;
	}

	std::vector<Components::Decoration>& ComponentsHolder::foregroundDecorations()
	{
		return foregroundDecorations_;
	}

	std::unordered_map<::ComponentId, Components::Decoration>& ComponentsHolder::dynamicForegroundDecorations()
	{
		return dynamicForegroundDecorations_;
	}

	std::unordered_map<::ComponentId, Components::Missile>& ComponentsHolder::missiles()
	{
		return missiles_;
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
