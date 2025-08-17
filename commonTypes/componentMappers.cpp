#include "componentMappers.hpp"

#include <globals/components.hpp>

#include <components/Texture.hpp>
#include <components/AnimatedTexture.hpp>
#include <components/BlendingTexture.hpp>
#include <components/RenderTexture.hpp>
#include <components/Decoration.hpp>
#include <components/Actor.hpp>
#include <components/Grapple.hpp>
#include <components/Missile.hpp>
#include <components/Plane.hpp>
#include <components/Wall.hpp>
#include <components/Polyline.hpp>
#include <components/Shockwave.hpp>
#include <components/SoundBuffer.hpp>
#include <components/Sound.hpp>

namespace ComponentMappers
{
	Texture::Texture(Components::Texture& component, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(&component),
		componentId(component.getComponentId()),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	Texture::Texture(ComponentId id, bool static_, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(static_ ? &Globals::Components().staticTextures()[id] : &Globals::Components().textures()[id]),
		componentId(id),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	bool Texture::operator==(const Texture& other) const
	{
		return component == other.component;
	}

	bool Texture::operator!=(const Texture& other) const
	{
		return component != other.component;
	}

	bool Texture::operator<(const Texture& other) const
	{
		return component < other.component;
	}

	Texture& Texture::operator=(Components::Texture& component)
	{
		this->component = &component;
		componentId = component.getComponentId();
		return *this;
	}

	bool Texture::isValid() const
	{
		return component;
	}

	bool Texture::isStatic() const
	{
		assert(component);
		return component->isStatic();
	}

	AnimatedTexture::AnimatedTexture(Components::AnimatedTexture& component, glm::vec2 translate, float rotate, glm::vec2 scale, float speedScale):
		component(&component),
		componentId(component.getComponentId()),
		translate(translate),
		rotate(rotate),
		scale(scale),
		speedScale(speedScale)
	{
	}

	AnimatedTexture::AnimatedTexture(ComponentId id, bool static_, glm::vec2 translate, float rotate, glm::vec2 scale, float speedScale):
		component(static_ ? &Globals::Components().staticAnimatedTextures()[id] : &Globals::Components().animatedTextures()[id]),
		componentId(id),
		translate(translate),
		rotate(rotate),
		scale(scale),
		speedScale(speedScale)
	{
	}

	bool AnimatedTexture::operator==(const AnimatedTexture& other) const
	{
		return component == other.component;
	}

	bool AnimatedTexture::operator!=(const AnimatedTexture& other) const
	{
		return component != other.component;
	}

	bool AnimatedTexture::operator<(const AnimatedTexture& other) const
	{
		return component < other.component;
	}

	AnimatedTexture& AnimatedTexture::operator=(Components::AnimatedTexture& component)
	{
		this->component = &component;
		componentId = component.getComponentId();
		return *this;
	}

	bool AnimatedTexture::isValid() const
	{
		return component;
	}

	bool AnimatedTexture::isStatic() const
	{
		assert(component);
		return component->isStatic();
	}

	BlendingTexture::BlendingTexture(Components::BlendingTexture& component, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(&component),
		componentId(component.getComponentId()),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	BlendingTexture::BlendingTexture(ComponentId id, bool static_, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(static_ ? &Globals::Components().staticBlendingTextures()[id] : &Globals::Components().blendingTextures()[id]),
		componentId(id),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	bool BlendingTexture::operator==(const BlendingTexture& other) const
	{
		return component == other.component;
	}

	bool BlendingTexture::operator!=(const BlendingTexture& other) const
	{
		return component != other.component;
	}

	bool BlendingTexture::operator<(const BlendingTexture& other) const
	{
		return component < other.component;
	}

	BlendingTexture& BlendingTexture::operator=(Components::BlendingTexture& component)
	{
		this->component = &component;
		componentId = component.getComponentId();
		return *this;
	}

	bool BlendingTexture::isValid() const
	{
		return component;
	}

	bool BlendingTexture::isStatic() const
	{
		assert(component);
		return component->isStatic();
	}

	RenderTexture::RenderTexture(Components::RenderTexture& component, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(&component),
		componentId(component.getComponentId()),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	RenderTexture::RenderTexture(ComponentId id, bool static_, glm::vec2 translate, float rotate, glm::vec2 scale):
		component(static_ ? &Globals::Components().staticRenderTextures()[id] : &Globals::Components().renderTextures()[id]),
		componentId(id),
		translate(translate),
		rotate(rotate),
		scale(scale)
	{
	}

	bool RenderTexture::operator==(const RenderTexture& other) const
	{
		return component == other.component;
	}

	bool RenderTexture::operator!=(const RenderTexture& other) const
	{
		return component != other.component;
	}

	bool RenderTexture::operator<(const RenderTexture& other) const
	{
		return component < other.component;
	}

	RenderTexture& RenderTexture::operator=(Components::RenderTexture& component)
	{
		this->component = &component;
		componentId = component.getComponentId();
		return *this;
	}

	bool RenderTexture::isValid() const
	{
		return component;
	}

	bool RenderTexture::isStatic() const
	{
		assert(component);
		return component->isStatic();
	}

	Decoration::Decoration(Components::Decoration& component) :
		component(&component),
		componentId(component.getComponentId())
	{
	}

	Decoration::Decoration(ComponentId id, bool static_) :
		component(static_ ? &Globals::Components().staticDecorations()[id] : &Globals::Components().decorations()[id]),
		componentId(id)
	{
	}

	bool Decoration::operator==(const Decoration& other) const
	{
		return component == other.component;
	}

	bool Decoration::operator!=(const Decoration& other) const
	{
		return component != other.component;
	}

	bool Decoration::operator<(const Decoration& other) const
	{
		return component < other.component;
	}

	Decoration& Decoration::operator=(Components::Decoration& component)
	{
		this->component = &component;
		componentId = component.getComponentId();
		return *this;
	}

	bool Decoration::isValid() const
	{
		return component;
	}

	bool Decoration::isStatic() const
	{
		assert(component);
		return component->isStatic();
	}

	Actor::Actor(Components::Actor& component):
		component(&component),
		componentId(component.getComponentId())
	{
	}

	Actor::Actor(ComponentId id) :
		component(&Globals::Components().actors()[id]),
		componentId(id)
	{
	}

	bool Actor::operator==(const Actor& other) const
	{
		return component == other.component;
	}

	bool Actor::operator!=(const Actor& other) const
	{
		return component != other.component;
	}

	bool Actor::operator<(const Actor& other) const
	{
		return component < other.component;
	}

	Actor& Actor::operator=(Components::Actor& component)
	{
		this->component = &component;
		componentId = component.getComponentId();
		return *this;
	}

	bool Actor::isValid() const
	{
		return component;
	}

	bool Actor::isStatic() const
	{
		assert(component);
		return component->isStatic();
	}

	Grapple::Grapple(Components::Grapple& component):
		component(&component),
		componentId(component.getComponentId())
	{
	}

	Grapple::Grapple(ComponentId id, bool static_):
		component(static_ ? &Globals::Components().staticGrapples()[id] : &Globals::Components().grapples()[id]),
		componentId(id)
	{
	}

	bool Grapple::operator==(const Grapple& other) const
	{
		return component == other.component;
	}

	bool Grapple::operator!=(const Grapple& other) const
	{
		return component != other.component;
	}

	bool Grapple::operator<(const Grapple& other) const
	{
		return component < other.component;
	}

	Grapple& Grapple::operator=(Components::Grapple& component)
	{
		this->component = &component;
		componentId = component.getComponentId();
		return *this;
	}

	bool Grapple::isValid() const
	{
		return component;
	}

	bool Grapple::isStatic() const
	{
		assert(component);
		return component->isStatic();
	}

	Missile::Missile(Components::Missile& component):
		component(&component),
		componentId(component.getComponentId())
	{
	}

	Missile::Missile(ComponentId id):
		component(&Globals::Components().missiles()[id]),
		componentId(id)
	{
	}

	bool Missile::operator==(const Missile& other) const
	{
		return component == other.component;
	}

	bool Missile::operator!=(const Missile& other) const
	{
		return component != other.component;
	}

	bool Missile::operator<(const Missile& other) const
	{
		return component < other.component;
	}

	Missile& Missile::operator=(Components::Missile& component)
	{
		this->component = &component;
		componentId = component.getComponentId();
		return *this;
	}

	bool Missile::isValid() const
	{
		return component;
	}

	bool Missile::isStatic() const
	{
		assert(component);
		return component->isStatic();
	}

	Plane::Plane(Components::Plane& component):
		component(&component),
		componentId(component.getComponentId())
	{
	}

	Plane::Plane(ComponentId id):
		component(&Globals::Components().planes()[id]),
		componentId(id)
	{
	}

	bool Plane::operator==(const Plane& other) const
	{
		return component == other.component;
	}

	bool Plane::operator!=(const Plane& other) const
	{
		return component != other.component;
	}

	bool Plane::operator<(const Plane& other) const
	{
		return component < other.component;
	}

	Plane& Plane::operator=(Components::Plane& component)
	{
		this->component = &component;
		componentId = component.getComponentId();
		return *this;
	}

	bool Plane::isValid() const
	{
		return component;
	}

	bool Plane::isStatic() const
	{
		assert(component);
		return component->isStatic();
	}

	Wall::Wall(Components::Wall& component):
		component(&component),
		componentId(component.getComponentId())
	{
	}

	Wall::Wall(ComponentId id, bool static_):
		component(static_ ? &Globals::Components().staticWalls()[id] : &Globals::Components().walls()[id]),
		componentId(id)
	{
	}

	bool Wall::operator==(const Wall& other) const
	{
		return component == other.component;
	}

	bool Wall::operator!=(const Wall& other) const
	{
		return component != other.component;
	}

	bool Wall::operator<(const Wall& other) const
	{
		return component < other.component;
	}

	Wall& Wall::operator=(Components::Wall& component)
	{
		this->component = &component;
		componentId = component.getComponentId();
		return *this;
	}

	bool Wall::isValid() const
	{
		return component;
	}

	bool Wall::isStatic() const
	{
		assert(component);
		return component->isStatic();
	}

	Polyline::Polyline(Components::Polyline& component):
		component(&component),
		componentId(component.getComponentId())
	{
	}

	Polyline::Polyline(ComponentId id, bool static_):
		component(static_ ? &Globals::Components().staticPolylines()[id] : &Globals::Components().polylines()[id]),
		componentId(id)
	{
	}

	bool Polyline::operator==(const Polyline& other) const
	{
		return component == other.component;
	}

	bool Polyline::operator!=(const Polyline& other) const
	{
		return component != other.component;
	}

	bool Polyline::operator<(const Polyline& other) const
	{
		return component < other.component;
	}

	Polyline& Polyline::operator=(Components::Polyline& component)
	{
		this->component = &component;
		componentId = component.getComponentId();
		return *this;
	}

	bool Polyline::isValid() const
	{
		return component;
	}

	bool Polyline::isStatic() const
	{
		assert(component);
		return component->isStatic();
	}

	ShockwaveParticle::ShockwaveParticle(Components::Shockwave& component) :
		component(&component),
		componentId(component.getComponentId())
	{
	}

	ShockwaveParticle::ShockwaveParticle(ComponentId id) :
		component(&Globals::Components().shockwaves()[id]),
		componentId(id)
	{
	}

	bool ShockwaveParticle::operator==(const ShockwaveParticle& other) const
	{
		return component == other.component;
	}

	bool ShockwaveParticle::operator!=(const ShockwaveParticle& other) const
	{
		return component != other.component;
	}

	bool ShockwaveParticle::operator<(const ShockwaveParticle& other) const
	{
		return component < other.component;
	}

	ShockwaveParticle& ShockwaveParticle::operator=(Components::Shockwave& component)
	{
		this->component = &component;
		componentId = component.getComponentId();
		return *this;
	}

	bool ShockwaveParticle::isValid() const
	{
		return component;
	}

	bool ShockwaveParticle::isStatic() const
	{
		assert(component);
		return component->isStatic();
	}

	SoundBuffer::SoundBuffer(Components::SoundBuffer& component) :
		component(&component),
		componentId(component.getComponentId())
	{
	}

	SoundBuffer::SoundBuffer(ComponentId id, bool static_) :
		component(static_ ? &Globals::Components().staticSoundsBuffers()[id] : &Globals::Components().soundsBuffers()[id]),
		componentId(id)
	{
	}

	bool SoundBuffer::operator==(const SoundBuffer& other) const
	{
		return component == other.component;
	}

	bool SoundBuffer::operator!=(const SoundBuffer& other) const
	{
		return component != other.component;
	}

	bool SoundBuffer::operator<(const SoundBuffer& other) const
	{
		return component < other.component;
	}

	SoundBuffer& SoundBuffer::operator=(Components::SoundBuffer& component)
	{
		this->component = &component;
		componentId = component.getComponentId();
		return *this;
	}

	bool SoundBuffer::isValid() const
	{
		return component;
	}

	bool SoundBuffer::isStatic() const
	{
		assert(component);
		return component->isStatic();
	}

	Sound::Sound(Components::Sound& component) :
		component(&component),
		componentId(component.getComponentId())
	{
	}

	Sound::Sound(ComponentId id, bool static_) :
		component(&Globals::Components().sounds()[id]),
		componentId(id)
	{
	}

	bool Sound::operator==(const Sound& other) const
	{
		return component == other.component;
	}

	bool Sound::operator!=(const Sound& other) const
	{
		return component != other.component;
	}

	bool Sound::operator<(const Sound& other) const
	{
		return component < other.component;
	}

	Sound& Sound::operator=(Components::Sound& component)
	{
		this->component = &component;
		componentId = component.getComponentId();
		return *this;
	}

	bool Sound::isValid() const
	{
		return component;
	}

	bool Sound::isStatic() const
	{
		assert(component);
		return component->isStatic();
	}
}
