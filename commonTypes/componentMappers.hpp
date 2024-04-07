#pragma once

#include "componentId.hpp"

#include <glm/vec2.hpp>

#include <variant>

namespace Components
{
	struct Texture;
	struct AnimatedTexture;
	struct BlendingTexture;
	struct Grapple;
	struct Missile;
	struct Plane;
	struct StaticWall;
	struct DynamicWall;
	struct StaticPolyline;
	struct DynamicPolyline;
}

namespace ComponentMappers
{
	struct StaticTexture
	{
		StaticTexture() = default;
		StaticTexture(Components::Texture* component, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });
		StaticTexture(ComponentId id, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

		Components::Texture* component;
		ComponentId componentId;

		glm::vec2 translate;
		float rotate;
		glm::vec2 scale;

		bool operator==(const StaticTexture&) const = default;
	};

	struct DynamicTexture
	{
		DynamicTexture(Components::Texture* component, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });
		DynamicTexture(ComponentId id, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

		Components::Texture* component;
		ComponentId componentId;

		glm::vec2 translate;
		float rotate;
		glm::vec2 scale;

		bool operator==(const DynamicTexture&) const = default;
	};

	struct StaticAnimatedTexture
	{
		StaticAnimatedTexture(Components::AnimatedTexture* component, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });
		StaticAnimatedTexture(ComponentId id, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

		Components::AnimatedTexture* component;
		ComponentId componentId;

		glm::vec2 translate;
		float rotate;
		glm::vec2 scale;

		bool operator==(const StaticAnimatedTexture&) const = default;
	};

	struct DynamicAnimatedTexture
	{
		DynamicAnimatedTexture(Components::AnimatedTexture* component, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });
		DynamicAnimatedTexture(ComponentId id, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

		Components::AnimatedTexture* component;
		ComponentId componentId;

		glm::vec2 translate;
		float rotate;
		glm::vec2 scale;

		bool operator==(const DynamicAnimatedTexture&) const = default;
	};

	struct StaticBlendingTexture
	{
		StaticBlendingTexture(Components::BlendingTexture* component, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });
		StaticBlendingTexture(ComponentId id, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

		Components::BlendingTexture* component;
		ComponentId componentId;

		glm::vec2 translate;
		float rotate;
		glm::vec2 scale;

		bool operator==(const StaticBlendingTexture&) const = default;
	};

	struct DynamicBlendingTexture
	{
		DynamicBlendingTexture(Components::BlendingTexture* component, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });
		DynamicBlendingTexture(ComponentId id, glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

		Components::BlendingTexture* component;
		ComponentId componentId;

		glm::vec2 translate;
		float rotate;
		glm::vec2 scale;

		bool operator==(const DynamicBlendingTexture&) const = default;
	};

	struct Grapple
	{
		Grapple(Components::Grapple* component);
		Grapple(ComponentId id);

		Components::Grapple* component;
		ComponentId componentId;

		bool operator==(const Grapple&) const = default;
	};

	struct Missile
	{
		Missile(Components::Missile* component);
		Missile(ComponentId id);

		Components::Missile* component;
		ComponentId componentId;

		bool operator==(const Missile&) const = default;
	};

	struct Plane
	{
		Plane(Components::Plane* component);
		Plane(ComponentId id);

		Components::Plane* component;
		ComponentId componentId;

		bool operator==(const Plane&) const = default;
	};

	struct StaticWall
	{
		StaticWall(Components::StaticWall* component);
		StaticWall(ComponentId id);

		Components::StaticWall* component;
		ComponentId componentId;

		bool operator==(const StaticWall&) const = default;
	};

	struct DynamicWall
	{
		DynamicWall(Components::DynamicWall* component);
		DynamicWall(ComponentId id);

		Components::DynamicWall* component;
		ComponentId componentId;

		bool operator==(const DynamicWall&) const = default;
	};

	struct StaticPolyline
	{
		StaticPolyline(Components::StaticPolyline* component);
		StaticPolyline(ComponentId id);

		Components::StaticPolyline* component;
		ComponentId componentId;

		bool operator==(const StaticPolyline&) const = default;
	};

	struct DynamicPolyline
	{
		DynamicPolyline(Components::DynamicPolyline* component);
		DynamicPolyline(ComponentId id);

		Components::DynamicPolyline* component;
		ComponentId componentId;

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
	CM::DynamicBlendingTexture>;

using BodyComponentVariant = std::variant<
	std::monostate,
	CM::Grapple,
	CM::Missile,
	CM::Plane,
	CM::StaticWall,
	CM::DynamicWall,
	CM::StaticPolyline,
	CM::DynamicPolyline>;
