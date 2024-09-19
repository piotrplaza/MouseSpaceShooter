#pragma once

#include "componentId.hpp"

#include <glm/vec2.hpp>

#include <variant>

namespace Components
{
	struct Texture;
	struct AnimatedTexture;
	struct BlendingTexture;
	struct Actor;
	struct Grapple;
	struct Missile;
	struct Plane;
	struct Wall;
	struct Polyline;
}

namespace ComponentMappers
{
	struct StaticTexture
	{
		StaticTexture() = default;
		StaticTexture(Components::Texture* component, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });
		StaticTexture(ComponentId id, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

		Components::Texture* component = nullptr;
		ComponentId componentId = 0;

		glm::vec2 translate{};
		float rotate{};
		glm::vec2 scale{};

		bool operator==(const StaticTexture&) const = default;
	};

	struct DynamicTexture
	{
		DynamicTexture() = default;
		DynamicTexture(Components::Texture* component, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });
		DynamicTexture(ComponentId id, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

		Components::Texture* component = nullptr;
		ComponentId componentId = 0;

		glm::vec2 translate{};
		float rotate{};
		glm::vec2 scale{};

		bool operator==(const DynamicTexture&) const = default;
	};

	struct StaticAnimatedTexture
	{
		StaticAnimatedTexture() = default;
		StaticAnimatedTexture(Components::AnimatedTexture* component, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });
		StaticAnimatedTexture(ComponentId id, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

		Components::AnimatedTexture* component = nullptr;
		ComponentId componentId = 0;

		glm::vec2 translate{};
		float rotate{};
		glm::vec2 scale{};

		bool operator==(const StaticAnimatedTexture&) const = default;
	};

	struct DynamicAnimatedTexture
	{
		DynamicAnimatedTexture() = default;
		DynamicAnimatedTexture(Components::AnimatedTexture* component, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });
		DynamicAnimatedTexture(ComponentId id, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

		Components::AnimatedTexture* component = nullptr;
		ComponentId componentId = 0;

		glm::vec2 translate{};
		float rotate{};
		glm::vec2 scale{};

		bool operator==(const DynamicAnimatedTexture&) const = default;
	};

	struct StaticBlendingTexture
	{
		StaticBlendingTexture() = default;
		StaticBlendingTexture(Components::BlendingTexture* component, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });
		StaticBlendingTexture(ComponentId id, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

		Components::BlendingTexture* component = nullptr;
		ComponentId componentId = 0;

		glm::vec2 translate{};
		float rotate{};
		glm::vec2 scale{};

		bool operator==(const StaticBlendingTexture&) const = default;
	};

	struct DynamicBlendingTexture
	{
		DynamicBlendingTexture() = default;
		DynamicBlendingTexture(Components::BlendingTexture* component, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });
		DynamicBlendingTexture(ComponentId id, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

		Components::BlendingTexture* component = nullptr;
		ComponentId componentId = 0;

		glm::vec2 translate{};
		float rotate{};
		glm::vec2 scale{};

		bool operator==(const DynamicBlendingTexture&) const = default;
	};

	struct DummyTexture {};

	struct Actor
	{
		Actor() = default;
		Actor(Components::Actor* component);
		Actor(ComponentId id);

		Components::Actor* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const Actor&) const = default;
	};

	struct Grapple
	{
		Grapple() = default;
		Grapple(Components::Grapple* component);
		Grapple(ComponentId id);

		Components::Grapple* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const Grapple&) const = default;
	};

	struct Missile
	{
		Missile() = default;
		Missile(Components::Missile* component);
		Missile(ComponentId id);

		Components::Missile* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const Missile&) const = default;
	};

	struct Plane
	{
		Plane() = default;
		Plane(Components::Plane* component);
		Plane(ComponentId id);

		Components::Plane* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const Plane&) const = default;
	};

	struct StaticWall
	{
		StaticWall() = default;
		StaticWall(Components::Wall* component);
		StaticWall(ComponentId id);

		Components::Wall* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const StaticWall&) const = default;
	};

	struct DynamicWall
	{
		DynamicWall() = default;
		DynamicWall(Components::Wall* component);
		DynamicWall(ComponentId id);

		Components::Wall* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const DynamicWall&) const = default;
	};

	struct StaticPolyline
	{
		StaticPolyline() = default;
		StaticPolyline(Components::Polyline* component);
		StaticPolyline(ComponentId id);

		Components::Polyline* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const StaticPolyline&) const = default;
	};

	struct DynamicPolyline
	{
		DynamicPolyline() = default;
		DynamicPolyline(Components::Polyline* component);
		DynamicPolyline(ComponentId id);

		Components::Polyline* component = nullptr;
		ComponentId componentId = 0;

		bool operator==(const DynamicPolyline&) const = default;
	};
}

namespace CM = ComponentMappers;

using TextureComponentVariant = std::variant<
	std::monostate,
	CM::StaticTexture,
	CM::DynamicTexture>;

using AbstractTextureComponentVariant = std::variant<
	std::monostate,
	CM::StaticTexture,
	CM::DynamicTexture,
	CM::StaticAnimatedTexture,
	CM::DynamicAnimatedTexture,
	CM::StaticBlendingTexture,
	CM::DynamicBlendingTexture,
	CM::DummyTexture>;

using BodyComponentVariant = std::variant<
	std::monostate,
	CM::Grapple,
	CM::Missile,
	CM::Actor,
	CM::Plane,
	CM::StaticWall,
	CM::DynamicWall,
	CM::StaticPolyline,
	CM::DynamicPolyline>;
