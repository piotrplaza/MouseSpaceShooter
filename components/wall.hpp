#pragma once

#include <memory>
#include <vector>

#include <Box2D/Box2D.h>

#include <tools/b2Helpers.hpp>

namespace Components
{
	struct Wall
	{
		Wall(std::unique_ptr<b2Body, b2BodyDeleter> body): body(std::move(body))
		{
		}

		std::unique_ptr<b2Body, b2BodyDeleter> body;
		std::vector<glm::vec3> verticesCache;

		void updateVerticesCache()
		{
			const auto& bodyTransform = body->GetTransform();
			const auto modelMatrix = glm::rotate(glm::translate(glm::mat4(1.0f), { bodyTransform.p.x, bodyTransform.p.y, 0.0f }),
				bodyTransform.q.GetAngle(), { 0.0f, 0.0f, 1.0f });

			const auto& fixture = *body->GetFixtureList();
			assert(!fixture.GetNext());
			assert(fixture.GetType() == b2Shape::e_polygon); //Temporary. TODO: Add other shapes.

			const auto& polygonShape = static_cast<const b2PolygonShape&>(*fixture.GetShape());

			verticesCache.clear();
			assert(polygonShape.m_count == 4); //Temporary. TODO: Add triangulation.
			verticesCache.reserve(6);
			for (int i = 0; i < 6; ++i)
			{
				const auto& b2v = polygonShape.m_vertices[i < 3 ? i : (i - 1) % 4];
				verticesCache.push_back(modelMatrix * glm::vec4(b2v.x, b2v.y, 0.0f, 1.0f));
			}
		}
	};
}
