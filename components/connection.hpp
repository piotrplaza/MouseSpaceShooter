#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace Components
{
	struct Connection
	{
		Connection(const glm::vec2& p1, const glm::vec2& p2, const glm::vec4& color): p1(p1), p2(p2), color(color)
		{
		}

		glm::vec2 p1;
		glm::vec2 p2;
		
		glm::vec4 color;

		std::vector<glm::vec3> verticesCache;
		std::vector<glm::vec4> colorsCache;

		void updateVerticesCache()
		{
			verticesCache.clear();
			verticesCache.reserve(2);
			verticesCache.emplace_back(p1, 0.0f);
			verticesCache.emplace_back(p2, 0.0f);
		}

		void updateColorsCache()
		{
			colorsCache.clear();
			colorsCache.reserve(2);
			colorsCache.push_back(color);
			colorsCache.push_back(color);
		}
	};
}
