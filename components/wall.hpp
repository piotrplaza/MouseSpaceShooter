#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <functional>

#include <Box2D/Box2D.h>

#include <ogl/shaders.hpp>

#include <globals.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/graphicsHelpers.hpp>

namespace Components
{
	struct Wall
	{
		Wall(std::unique_ptr<b2Body, b2BodyDeleter> body, std::optional<unsigned> texture = std::nullopt,
			std::function<std::function<void()>(Shaders::ProgramId)> renderingSetup = nullptr):
			body(std::move(body)),
			texture(texture),
			renderingSetup(std::move(renderingSetup))
		{
		}

		std::unique_ptr<b2Body, b2BodyDeleter> body;
		std::optional<unsigned> texture;
		std::function<std::function<void()>(Shaders::ProgramId)> renderingSetup;

		std::vector<glm::vec3> getPositionsCache() const
		{
			using namespace Globals::Constants;

			std::vector<glm::vec3> positionsCache;

			const auto& fixture = *body->GetFixtureList();
			assert(!fixture.GetNext());

			switch (fixture.GetType())
			{
			case b2Shape::e_polygon:
			{
				const auto& polygonShape = static_cast<const b2PolygonShape&>(*fixture.GetShape());
				positionsCache.clear();
				assert(polygonShape.m_count == 4); //Temporary. TODO: Add triangulation.
				positionsCache.reserve(6);
				for (int i = 0; i < 6; ++i)
				{
					const auto& b2v = polygonShape.m_vertices[i < 3 ? i : (i - 1) % 4];
					positionsCache.emplace_back(b2v.x, b2v.y, 0.0f);
				}
			} break;
			case b2Shape::e_circle:
			{
				const auto& circleShape = static_cast<const b2CircleShape&>(*fixture.GetShape());
				positionsCache = Tools::CreateCirclePositions(ToVec2<glm::vec2>(circleShape.m_p), circleShape.m_radius, circleGraphicsComplexity);
			} break;
			default:
				assert(!"unsupported shape type");
			}

			return positionsCache;
		}

		std::vector<glm::vec3> getTransformedPositionsCache() const
		{
			const auto modelMatrix = Tools::GetModelMatrix(*body);
			auto transformedPositionsCache = getPositionsCache();

			for (auto& position : transformedPositionsCache) position = modelMatrix * glm::vec4(position, 1.0f);

			return transformedPositionsCache;
		}
	};
}
