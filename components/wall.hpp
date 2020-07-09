#pragma once

#include <memory>
#include <vector>
#include <optional>

#include <glm/gtx/transform.hpp>

#include <Box2D/Box2D.h>

#include <globals.hpp>
#include <tools/b2Helpers.hpp>
#include <tools/graphicsHelpers.hpp>

namespace Components
{
	struct Wall
	{
		Wall(std::unique_ptr<b2Body, b2BodyDeleter> body, std::optional<unsigned> texture = std::nullopt):
			body(std::move(body)),
			texture(texture)
		{
		}

		std::unique_ptr<b2Body, b2BodyDeleter> body;
		std::optional<unsigned> texture;

		std::vector<glm::vec3> generateVerticesCache(bool transform = true)
		{
			using namespace Globals::Constants;

			std::vector<glm::vec3> verticesCache;

			const auto& bodyTransform = body->GetTransform();
			const auto modelMatrix = transform
				? glm::rotate(glm::translate(glm::mat4(1.0f), { bodyTransform.p.x, bodyTransform.p.y, 0.0f }),
					bodyTransform.q.GetAngle(), { 0.0f, 0.0f, 1.0f })
				: glm::mat4(1.0f);

			const auto& fixture = *body->GetFixtureList();
			assert(!fixture.GetNext());

			switch (fixture.GetType())
			{
			case b2Shape::e_polygon:
			{
				const auto& polygonShape = static_cast<const b2PolygonShape&>(*fixture.GetShape());
				verticesCache.clear();
				assert(polygonShape.m_count == 4); //Temporary. TODO: Add triangulation.
				verticesCache.reserve(6);
				for (int i = 0; i < 6; ++i)
				{
					const auto& b2v = polygonShape.m_vertices[i < 3 ? i : (i - 1) % 4];
					verticesCache.push_back(modelMatrix * glm::vec4(b2v.x, b2v.y, 0.0f, 1.0f));
				}
			} break;
			case b2Shape::e_circle:
			{
				const auto& circleShape = static_cast<const b2CircleShape&>(*fixture.GetShape());
				verticesCache = Tools::CreateCircleVertices(ToVec2<glm::vec2>(circleShape.m_p), circleShape.m_radius, circleGraphicsComplexity, modelMatrix);
			} break;
			default:
				assert(!"unsupported shape type");
			}

			return verticesCache;
		}
	};
}
