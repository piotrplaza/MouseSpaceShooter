#pragma once

#include "common.hpp"

#include <commonTypes/fTypes.hpp>

#include <functional>
#include <optional>
#include <fstream>
#include <unordered_map>
#include <list>

namespace Levels
{
	class SplineEditing
	{
	public:
		SplineEditing(const glm::vec2& mousePos, const glm::vec2& oldMousePos, const glm::vec2& mouseDelta, const float& zoomScale, FBool ongoing);

		void edit(bool& cameraMoving);
		void update() const;

		void generateCode(std::ofstream& fs) const;

	private:
		struct SplineDef
		{
			std::list<ControlPoint> controlPoints;
			bool lightning = false;
			bool loop = false;
			int complexity = 10;
		};

		const glm::vec2& mousePos;
		const glm::vec2& oldMousePos;
		const glm::vec2& mouseDelta;

		const float& zoomScale;

		FBool ongoing;

		std::unordered_map<ComponentId, SplineDef> splineDecorationIdToSplineDef;
		std::optional<ComponentId> activeSplineDecorationId;
		std::optional<std::list<ControlPoint>::iterator> movingControlPoint;
		std::optional<std::list<ControlPoint>::iterator> movingAdjacentControlPoint;
	};
}
