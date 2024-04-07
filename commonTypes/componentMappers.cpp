#include "componentMappers.hpp"

#include <globals/components.hpp>

#include <components/Texture.hpp>
#include <components/AnimatedTexture.hpp>
#include <components/BlendingTexture.hpp>
#include <components/Grapple.hpp>
#include <components/Missile.hpp>
#include <components/Plane.hpp>
#include <components/Wall.hpp>
#include <components/Polyline.hpp>

namespace ComponentMappers
{
	StaticTexture::StaticTexture(Components::Texture* component, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(component),
		componentId(component->getComponentId()),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	StaticTexture::StaticTexture(ComponentId id, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(&Globals::Components().staticTextures()[id]),
		componentId(id),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	DynamicTexture::DynamicTexture(Components::Texture* component, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(component),
		componentId(component->getComponentId()),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	DynamicTexture::DynamicTexture(ComponentId id, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(&Globals::Components().dynamicTextures()[id]),
		componentId(id),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	StaticAnimatedTexture::StaticAnimatedTexture(Components::AnimatedTexture* component, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(component),
		componentId(component->getComponentId()),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	StaticAnimatedTexture::StaticAnimatedTexture(ComponentId id, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(&Globals::Components().staticAnimatedTextures()[id]),
		componentId(id),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	DynamicAnimatedTexture::DynamicAnimatedTexture(Components::AnimatedTexture* component, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(component),
		componentId(component->getComponentId()),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	DynamicAnimatedTexture::DynamicAnimatedTexture(ComponentId id, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(&Globals::Components().dynamicAnimatedTextures()[id]),
		componentId(id),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	StaticBlendingTexture::StaticBlendingTexture(Components::BlendingTexture* component, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(component),
		componentId(component->getComponentId()),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	StaticBlendingTexture::StaticBlendingTexture(ComponentId id, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(&Globals::Components().staticBlendingTextures()[id]),
		componentId(id),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	DynamicBlendingTexture::DynamicBlendingTexture(Components::BlendingTexture* component, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(component),
		componentId(component->getComponentId()),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	DynamicBlendingTexture::DynamicBlendingTexture(ComponentId id, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(&Globals::Components().dynamicBlendingTextures()[id]),
		componentId(id),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	Grapple::Grapple(Components::Grapple* component):
		component(component),
		componentId(component->getComponentId())
	{
	}

	Grapple::Grapple(ComponentId id):
		component(&Globals::Components().grapples()[id]),
		componentId(id)
	{
	}

	Missile::Missile(Components::Missile* component):
		component(component),
		componentId(component->getComponentId())
	{
	}

	Missile::Missile(ComponentId id):
		component(&Globals::Components().missiles()[id]),
		componentId(id)
	{
	}

	Plane::Plane(Components::Plane* component):
		component(component),
		componentId(component->getComponentId())
	{
	}

	Plane::Plane(ComponentId id):
		component(&Globals::Components().planes()[id]),
		componentId(id)
	{
	}

	StaticWall::StaticWall(Components::Wall* component):
		component(component),
		componentId(component->getComponentId())
	{
	}

	StaticWall::StaticWall(ComponentId id):
		component(&Globals::Components().staticWalls()[id]),
		componentId(id)
	{
	}

	DynamicWall::DynamicWall(Components::Wall* component):
		component(component),
		componentId(component->getComponentId())
	{
	}

	DynamicWall::DynamicWall(ComponentId id):
		component(&Globals::Components().dynamicWalls()[id]),
		componentId(id)
	{
	}

	StaticPolyline::StaticPolyline(Components::Polyline* component):
		component(component),
		componentId(component->getComponentId())
	{
	}

	StaticPolyline::StaticPolyline(ComponentId id):
		component(&Globals::Components().staticPolylines()[id]),
		componentId(id)
	{
	}

	DynamicPolyline::DynamicPolyline(Components::Polyline* component):
		component(component),
		componentId(component->getComponentId())
	{
	}

	DynamicPolyline::DynamicPolyline(ComponentId id):
		component(&Globals::Components().dynamicPolylines()[id]),
		componentId(id)
	{
	}
}
