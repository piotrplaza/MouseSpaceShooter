#pragma once

#include "common.hpp"

#include <commonTypes/fTypes.hpp>

#include <functional>
#include <optional>
#include <fstream>
#include <unordered_map>
#include <list>

namespace Tools
{
	class ParamsFromFile;
}


namespace Levels
{
	class SplineEditing
	{
	public:
		SplineEditing(const glm::vec2& mousePos, const glm::vec2& oldMousePos, const glm::vec2& mouseDelta, const float& zoomScale, FBool ongoing, const Tools::ParamsFromFile& paramsFromFile);

		void edit();
		void update() const;

		void generateCode(std::ofstream& fs) const;

	private:
		struct SplineDef
		{
			std::list<ControlPoint> controlPoints;
			bool loop = false;
			int complexity = 10;
			int lightning = 0;
		};

		const glm::vec2& mousePos;
		const glm::vec2& oldMousePos;
		const glm::vec2& mouseDelta;

		const float& zoomScale;

		FBool ongoing;

		const Tools::ParamsFromFile& paramsFromFile;

		std::unordered_map<ComponentId, SplineDef> polylineIdToSplineDef;
		std::optional<ComponentId> activePolylineId;
		std::optional<std::list<ControlPoint>::iterator> movingControlPoint;
		std::optional<std::list<ControlPoint>::iterator> movingAdjacentControlPoint;
		float controlPointRadius = 0.75f;
		glm::vec4 activeSplineColor = { 0.0f, 0.0f, 1.0f, 1.0f };
		glm::vec4 inactiveSplineColor;
	};
}
