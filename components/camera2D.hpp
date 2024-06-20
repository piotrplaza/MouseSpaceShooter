#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>

#include <commonTypes/fTypes.hpp>

namespace Components
{
	struct Camera2D : ComponentBase
	{
		FVec2 targetPositionF = glm::vec2(0.0f, 0.0f);
		FFloat targetProjectionHSizeF = 10.0f;

		float positionTransitionFactor = 1.0f;
		float projectionTransitionFactor = 1.0f;

		struct
		{
			glm::vec2 position{ 0.0f };
			glm::vec2 prevPosition{ 0.0f };

			float projectionHSize = 0.0f;
			float prevProjectionHSize = 0.0f;
		} details;

		std::function<float(float)> projectionHSizeToZ = [](float projectionHSize) { return projectionHSize * 0.5f; };

		float getZ() const
		{
			if (!projectionHSizeToZ)
				return 0.0f;

			return projectionHSizeToZ(details.projectionHSize);
		}
	};
}
