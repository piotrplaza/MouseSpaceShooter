#pragma once

#include "components/wall.hpp"
#include "typeComponentMappers.hpp"

#include <tools/graphicsHelpers.hpp>
#include <tools/b2Helpers.hpp>

#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/bodyUserData.hpp>

#include <ogl/shaders.hpp>

#include <Box2D/Box2D.h>

#include <glm/glm.hpp>

#include <vector>
#include <optional>
#include <functional>
#include <variant>

namespace Components
{
	struct Grapple : Wall
	{
		using Wall::Wall;

		Grapple(Body body,
			TextureComponentVariant texture = std::monostate{},
			std::optional<ComponentId>  renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			Wall(std::move(body), texture, renderingSetup, renderLayer, customShadersProgram)
		{
			Tools::AccessUserData(*this->body).bodyComponentVariant = TCM::Grapple(getComponentId());
		}

		float influenceRadius = 0.0f;

		struct
		{
			glm::vec2 previousCenter{ 0.0f, 0.0f };
		} details;
	};
}
