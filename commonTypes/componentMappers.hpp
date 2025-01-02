#pragma once

#include "componentId.hpp"

#include <glm/vec2.hpp>

#include <variant>

namespace Components
{
	struct Texture;
	struct AnimatedTexture;
	struct BlendingTexture;
	struct Decoration;
	struct Actor;
	struct Grapple;
	struct Missile;
	struct Plane;
	struct Wall;
	struct Polyline;
	struct Shockwave;
	struct SoundBuffer;
	struct Sound;
}

namespace ComponentMappers
{
	struct Texture
	{
		Texture() = default;
		Texture(Components::Texture& component, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });
		Texture(ComponentId id, bool static_, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

		Components::Texture* component = nullptr;
		ComponentId componentId = 0;

		glm::vec2 translate{};
		float rotate{};
		glm::vec2 scale{ 1.0f, 1.0f };

		bool operator==(const Texture&) const;
		bool operator!=(const Texture&) const;
		bool operator<(const Texture&) const;
		Texture& operator=(Components::Texture& component);

		bool isValid() const;
		bool isStatic() const;
	};

	struct AnimatedTexture
	{
		AnimatedTexture() = default;
		AnimatedTexture(Components::AnimatedTexture& component, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });
		AnimatedTexture(ComponentId id, bool static_, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

		Components::AnimatedTexture* component = nullptr;
		ComponentId componentId = 0;

		glm::vec2 translate{};
		float rotate{};
		glm::vec2 scale{ 1.0f, 1.0f };

		bool operator==(const AnimatedTexture&) const;
		bool operator!=(const AnimatedTexture&) const;
		bool operator<(const AnimatedTexture&) const;
		AnimatedTexture& operator=(Components::AnimatedTexture& component);

		bool isValid() const;
		bool isStatic() const;
	};

	struct BlendingTexture
	{
		BlendingTexture() = default;
		BlendingTexture(Components::BlendingTexture& component, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });
		BlendingTexture(ComponentId id, bool static_, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

		Components::BlendingTexture* component = nullptr;
		ComponentId componentId = 0;

		glm::vec2 translate{};
		float rotate{};
		glm::vec2 scale{ 1.0f, 1.0f };

		bool operator==(const BlendingTexture&) const;
		bool operator!=(const BlendingTexture&) const;
		bool operator<(const BlendingTexture&) const;
		BlendingTexture& operator=(Components::BlendingTexture& component);

		bool isValid() const;
		bool isStatic() const;
	};

	struct DummyTexture {};

	struct Decoration
	{
		Decoration() = default;
		Decoration(Components::Decoration& component);
		Decoration(ComponentId id, bool static_);

		Components::Decoration* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const Decoration&) const;
		bool operator!=(const Decoration&) const;
		bool operator<(const Decoration&) const;
		Decoration& operator=(Components::Decoration& component);

		bool isValid() const;
		bool isStatic() const;
	};

	struct Actor
	{
		Actor() = default;
		Actor(Components::Actor& component);
		Actor(ComponentId id);

		Components::Actor* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const Actor&) const;
		bool operator!=(const Actor&) const;
		bool operator<(const Actor&) const;
		Actor& operator=(Components::Actor& component);

		bool isValid() const;
		bool isStatic() const;
	};

	struct Grapple
	{
		Grapple() = default;
		Grapple(Components::Grapple& component);
		Grapple(ComponentId id);

		Components::Grapple* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const Grapple&) const;
		bool operator!=(const Grapple&) const;
		bool operator<(const Grapple&) const;
		Grapple& operator=(Components::Grapple& component);

		bool isValid() const;
		bool isStatic() const;
	};

	struct Missile
	{
		Missile() = default;
		Missile(Components::Missile& component);
		Missile(ComponentId id);

		Components::Missile* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const Missile&) const;
		bool operator!=(const Missile&) const;
		bool operator<(const Missile&) const;
		Missile& operator=(Components::Missile& component);

		bool isValid() const;
		bool isStatic() const;
	};

	struct Plane
	{
		Plane() = default;
		Plane(Components::Plane& component);
		Plane(ComponentId id);

		Components::Plane* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const Plane&) const;
		bool operator!=(const Plane&) const;
		bool operator<(const Plane&) const;
		Plane& operator=(Components::Plane& component);

		bool isValid() const;
		bool isStatic() const;
	};

	struct Wall
	{
		Wall() = default;
		Wall(Components::Wall& component);
		Wall(ComponentId id, bool static_);

		Components::Wall* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const Wall&) const;
		bool operator!=(const Wall&) const;
		bool operator<(const Wall&) const;
		Wall& operator=(Components::Wall& component);

		bool isValid() const;
		bool isStatic() const;
	};

	struct Polyline
	{
		Polyline() = default;
		Polyline(Components::Polyline& component);
		Polyline(ComponentId id, bool static_);

		Components::Polyline* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const Polyline&) const;
		bool operator!=(const Polyline&) const;
		bool operator<(const Polyline&) const;
		Polyline& operator=(Components::Polyline& component);

		bool isValid() const;
		bool isStatic() const;
	};

	struct ShockwaveParticle
	{
		ShockwaveParticle() = default;
		ShockwaveParticle(Components::Shockwave& component);
		ShockwaveParticle(ComponentId id);

		Components::Shockwave* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const ShockwaveParticle&) const;
		bool operator!=(const ShockwaveParticle&) const;
		bool operator<(const ShockwaveParticle&) const;
		ShockwaveParticle& operator=(Components::Shockwave& component);

		bool isValid() const;
		bool isStatic() const;
	};

	struct SoundBuffer
	{
		SoundBuffer() = default;
		SoundBuffer(Components::SoundBuffer& component);
		SoundBuffer(ComponentId id, bool static_);

		Components::SoundBuffer* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const SoundBuffer&) const;
		bool operator!=(const SoundBuffer&) const;
		bool operator<(const SoundBuffer&) const;
		SoundBuffer& operator=(Components::SoundBuffer& component);

		bool isValid() const;
		bool isStatic() const;
	};

	struct Sound
	{
		Sound() = default;
		Sound(Components::Sound& component);
		Sound(ComponentId id, bool static_);

		Components::Sound* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const Sound&) const;
		bool operator!=(const Sound&) const;
		bool operator<(const Sound&) const;
		Sound& operator=(Components::Sound& component);

		bool isValid() const;
		bool isStatic() const;
	};
}

namespace CM = ComponentMappers;

namespace std {
	template<>
	struct hash<CM::Texture> {
		std::size_t operator()(const CM::Texture& texture) const {
			return (std::size_t)texture.component;
		}
	};

	template<>
	struct hash<CM::AnimatedTexture> {
		std::size_t operator()(const CM::AnimatedTexture& animatedTexture) const {
			return (std::size_t)animatedTexture.component;
		}
	};

	template<>
	struct hash<CM::BlendingTexture> {
		std::size_t operator()(const CM::BlendingTexture& blendingTexture) const {
			return (std::size_t)blendingTexture.component;
		}
	};

	template<>
	struct hash<CM::Decoration> {
		std::size_t operator()(const CM::Decoration& decoration) const {
			return (std::size_t)decoration.component;
		}
	};

	template<>
	struct hash<CM::Actor> {
		std::size_t operator()(const CM::Actor& actor) const {
			return (std::size_t)actor.component;
		}
	};

	template<>
	struct hash<CM::Grapple> {
		std::size_t operator()(const CM::Grapple& grapple) const {
			return (std::size_t)grapple.component;
		}
	};

	template<>
	struct hash<CM::Missile> {
		std::size_t operator()(const CM::Missile& missile) const {
			return (std::size_t)missile.component;
		}
	};

	template<>
	struct hash<CM::Plane> {
		std::size_t operator()(const CM::Plane& plane) const {
			return (std::size_t)plane.component;
		}
	};

	template<>
	struct hash<CM::Wall> {
		std::size_t operator()(const CM::Wall& wall) const {
			return (std::size_t)wall.component;
		}
	};

	template<>
	struct hash<CM::Polyline> {
		std::size_t operator()(const CM::Polyline& polyline) const {
			return (std::size_t)polyline.component;
		}
	};

	template<>
	struct hash<CM::SoundBuffer> {
		std::size_t operator()(const CM::SoundBuffer& soundBuffer) const {
			return (std::size_t)soundBuffer.component;
		}
	};

	template<>
	struct hash<CM::Sound> {
		std::size_t operator()(const CM::Sound& sound) const {
			return (std::size_t)sound.component;
		}
	};
}

using AbstractTextureComponentVariant = std::variant<
	std::monostate,
	CM::Texture,
	CM::AnimatedTexture,
	CM::BlendingTexture,
	CM::DummyTexture>;

using BodyComponentVariant = std::variant<
	std::monostate,
	CM::Grapple,
	CM::Missile,
	CM::Actor,
	CM::Plane,
	CM::Wall,
	CM::Polyline,
	CM::ShockwaveParticle>;
