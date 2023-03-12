#pragma once

#include "common.hpp"

#include <glm/vec2.hpp>

#include <functional>
#include <optional>
#include <fstream>
#include <list>
#include <array>

namespace Levels
{
	class StartingLineEditing
	{
	public:
		StartingLineEditing(const glm::vec2& mousePos, const glm::vec2& oldMousePos, const glm::vec2& mouseDelta, const float& zoomScale, std::function<bool()> ongoing);

		void edit(bool& cameraMoving);
		void update() const;

		void generateCode(std::ofstream& fs) const;

		std::array<glm::vec2, 2> getStartingLineEnds() const;
		float getStartingPositionLineDistance() const;

	private:
		const glm::vec2& mousePos;
		const glm::vec2& oldMousePos;
		const glm::vec2& mouseDelta;

		const float& zoomScale;

		std::function<bool()> ongoing;

		ComponentId startingLineId;
		ComponentId startingPositionLineId;
		std::array<ComponentId, 2> startingLineEnds;
		std::list<ControlPoint> controlPoints;
		std::optional<std::list<ControlPoint>::iterator> movingControlPoint;
		bool controlPointsEnds = false;
		float startingLineEndsRadius = 1.0f;
		float startingPositionLineDistance = 1.0f;
	};
}
