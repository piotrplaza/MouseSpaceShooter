#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <tools/graphicsHelpers.hpp>

namespace Components
{
	struct Connection
	{
		Connection(const glm::vec2& p1, const glm::vec2& p2, const glm::vec4& color, int segmentsNum = 1, float frayFactor = 0.5f)
			: p1(p1), p2(p2), color(color), segmentsNum(segmentsNum), frayFactor(frayFactor)
		{
		}

		glm::vec2 p1;
		glm::vec2 p2;
		
		glm::vec4 color;
		int segmentsNum;
		float frayFactor;

		std::vector<glm::vec3> verticesCache;
		std::vector<glm::vec4> colorsCache;

		void updateVerticesCache()
		{
			if (segmentsNum == 1)
			{
				verticesCache.clear();
				verticesCache.reserve(2);
				verticesCache.emplace_back(p1, 0.0f);
				verticesCache.emplace_back(p2, 0.0f);
			}
			else
			{
				verticesCache = tools::CreateLightningVertices(p1, p2, segmentsNum, frayFactor);
			}
		}

		void updateColorsCache()
		{
			colorsCache.clear();
			colorsCache.reserve(segmentsNum * 2);
			for (int i = 0; i < segmentsNum * 2; ++i) colorsCache.push_back(color);
		}
	};
}
