#pragma once

#include "componentId.hpp"

#include <glm/vec2.hpp>

#include <variant>

namespace Components
{
	struct Grapple;
	struct Missile;
	struct Plane;
	struct StaticWall;
	struct DynamicWall;
	struct StaticPolyline;
	struct DynamicPolyline;
}

namespace TypeComponentMappers
{
	struct StaticTexture
	{
		ComponentId id{};

		glm::vec2 translate = { 0.0f, 0.0f };
		float rotate = 0.0f;
		glm::vec2 scale = { 1.0f, 1.0f };

		bool operator==(const StaticTexture&) const = default;
	};

	struct DynamicTexture
	{
		ComponentId id{};

		glm::vec2 translate = { 0.0f, 0.0f };
		float rotate = 0.0f;
		glm::vec2 scale = { 1.0f, 1.0f };

		bool operator==(const DynamicTexture&) const = default;
	};

	struct StaticAnimatedTexture
	{
		ComponentId id{};

		glm::vec2 translate = { 0.0f, 0.0f };
		float rotate = 0.0f;
		glm::vec2 scale = { 1.0f, 1.0f };

		bool operator==(const StaticAnimatedTexture&) const = default;
	};

	struct DynamicAnimatedTexture
	{
		ComponentId id{};

		glm::vec2 translate = { 0.0f, 0.0f };
		float rotate = 0.0f;
		glm::vec2 scale = { 1.0f, 1.0f };

		bool operator==(const DynamicAnimatedTexture&) const = default;
	};

	struct StaticBlendingTexture
	{
		ComponentId id{};

		glm::vec2 translate = { 0.0f, 0.0f };
		float rotate = 0.0f;
		glm::vec2 scale = { 1.0f, 1.0f };

		bool operator==(const StaticBlendingTexture&) const = default;
	};

	struct DynamicBlendingTexture
	{
		ComponentId id{};

		glm::vec2 translate = { 0.0f, 0.0f };
		float rotate = 0.0f;
		glm::vec2 scale = { 1.0f, 1.0f };

		bool operator==(const DynamicBlendingTexture&) const = default;
	};

	struct Grapple
	{
		ComponentId id{};
		Components::Grapple* component = nullptr;

		bool operator==(const Grapple&) const = default;
	};

	struct Missile
	{
		ComponentId id;
		Components::Missile* component = nullptr;

		bool operator==(const Missile&) const = default;
	};

	struct Plane
	{
		ComponentId id{};
		Components::Plane* component = nullptr;

		bool operator==(const Plane&) const = default;
	};

	struct StaticWall
	{
		ComponentId id{};
		Components::StaticWall* component = nullptr;

		bool operator==(const StaticWall&) const = default;
	};

	struct DynamicWall
	{
		ComponentId id{};
		Components::DynamicWall* component = nullptr;

		bool operator==(const DynamicWall&) const = default;
	};

	struct StaticPolyline
	{
		ComponentId id{};
		Components::StaticPolyline* component = nullptr;

		bool operator==(const StaticPolyline&) const = default;
	};

	struct DynamicPolyline
	{
		ComponentId id{};
		Components::DynamicPolyline* component = nullptr;

		bool operator==(const DynamicPolyline&) const = default;
	};
}

namespace TCM = TypeComponentMappers;

using TextureComponentVariant = std::variant<
	std::monostate,
	TCM::StaticTexture,
	TCM::DynamicTexture>;

using AbstractTextureComponentVariant = std::variant<
	std::monostate,
	TCM::StaticTexture,
	TCM::DynamicTexture,
	TCM::StaticAnimatedTexture,
	TCM::DynamicAnimatedTexture,
	TCM::StaticBlendingTexture,
	TCM::DynamicBlendingTexture>;

using BodyComponentVariant = std::variant<
	std::monostate,
	TCM::Grapple,
	TCM::Missile,
	TCM::Plane,
	TCM::StaticWall,
	TCM::DynamicWall,
	TCM::StaticPolyline,
	TCM::DynamicPolyline>;
