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

		std::vector<glm::vec3> getPositions() const
		{
			std::vector<glm::vec3> positions;

			if (segmentsNum == 1)
			{
				positions.clear();
				positions.reserve(2);
				positions.emplace_back(p1, 0.0f);
				positions.emplace_back(p2, 0.0f);
			}
			else
			{
				positions = Tools::CreateLightningPositions(p1, p2, segmentsNum, frayFactor);
			}

			return positions;
		}

		std::vector<glm::vec4> getColors() const
		{
			std::vector<glm::vec4> colors;

			colors.clear();
			colors.reserve(segmentsNum * 2);
			for (int i = 0; i < segmentsNum * 2; ++i) colors.push_back(color);

			return colors;
		}
	};
}
