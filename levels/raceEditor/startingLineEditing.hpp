#pragma once

#include "common.hpp"

#include <commonTypes/fTypes.hpp>

#include <glm/vec2.hpp>

#include <functional>
#include <optional>
#include <fstream>
#include <list>
#include <array>

namespace Tools
{
	class ParamsFromFile;
}

namespace Levels
{
	class StartingLineEditing
	{
	public:
		StartingLineEditing(const glm::vec2& mousePos, const glm::vec2& oldMousePos, const glm::vec2& mouseDelta, const float& zoomScale, const float& scale, FBool ongoing, const Tools::ParamsFromFile& paramsFromFile);

		void edit();
		void update() const;

		void generateCode(std::ofstream& fs) const;

		std::vector<glm::vec2> getStartingLineEnds() const;
		float getStartingPositionLineDistance() const;

	private:
		const glm::vec2& mousePos;
		const glm::vec2& oldMousePos;
		const glm::vec2& mouseDelta;

		const float& zoomScale;
		const float& scale;

		FBool ongoing;

		const Tools::ParamsFromFile& paramsFromFile;

		ComponentId startingLineId;
		ComponentId startingPositionLineId;
		ComponentId arrowId;
		std::array<ComponentId, 2> startingLineEnds;
		std::list<ControlPoint> controlPoints;
		std::optional<std::list<ControlPoint>::iterator> movingControlPoint;
		bool controlPointsEnds = false;
		float startingLineEndsRadius = 1.0f;
		float startingPositionLineDistance = 1.0f;
		float controlPointRadius = 0.75f;
		glm::vec4 startingLineColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	};
}
