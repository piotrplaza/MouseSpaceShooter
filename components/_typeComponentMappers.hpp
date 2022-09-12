#pragma once

#include "_componentId.hpp"

#include <glm/vec2.hpp>

#include <variant>

namespace TypeComponentMappers
{
	struct Texture
	{
		ComponentId id;

		glm::vec2 translate = { 0.0f, 0.0f };
		float rotate = 0.0f;
		glm::vec2 scale = { 1.0f, 1.0f };

		bool operator==(const Texture&) const = default;
	};

	struct AnimatedTexture
	{
		ComponentId id;

		glm::vec2 translate = { 0.0f, 0.0f };
		float rotate = 0.0f;
		glm::vec2 scale = { 1.0f, 1.0f };

		bool operator==(const AnimatedTexture&) const = default;
	};

	struct BlendingTexture
	{
		ComponentId id;

		glm::vec2 translate = { 0.0f, 0.0f };
		float rotate = 0.0f;
		glm::vec2 scale = { 1.0f, 1.0f };

		bool operator==(const BlendingTexture&) const = default;
	};

	struct Grapple
	{
		ComponentId id;

		bool operator==(const Grapple&) const = default;
	};

	struct Missile
	{
		ComponentId id;

		bool operator==(const Missile&) const = default;
	};

	struct Plane
	{
		ComponentId id;

		bool operator==(const Plane&) const = default;
	};

	struct Wall
	{
		ComponentId id;

		bool operator==(const Wall&) const = default;
	};

	struct Polyline
	{
		ComponentId id;

		bool operator==(const Polyline&) const = default;
	};
}

namespace TCM = TypeComponentMappers;

using TextureComponentVariant = std::variant<std::monostate, TCM::Texture, TCM::AnimatedTexture, TCM::BlendingTexture>;
using BodyComponentVariant = std::variant<std::monostate, TCM::Grapple, TCM::Missile, TCM::Plane, TCM::Wall, TCM::Polyline>;
