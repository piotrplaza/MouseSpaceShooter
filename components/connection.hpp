#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <componentBase.hpp>

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
			if (segmentsNum == 1) return { { p1, 0.0f }, { p2, 0.0f } };
			else return Tools::CreatePositionsOfLightning(p1, p2, segmentsNum, frayFactor);
		}

		std::vector<glm::vec4> getColors() const
		{
			return std::vector<glm::vec4>(segmentsNum * 2, color);
		}
	};
}
