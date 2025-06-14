#pragma once

#include "common.hpp"

#include <commonTypes/fTypes.hpp>

#include <glm/vec2.hpp>

#include <fstream>
#include <optional>
#include <list>
#include <unordered_map>

namespace Levels
{
	class GrappleEditing
	{
	public:
		GrappleEditing(const glm::vec2& mousePos, const glm::vec2& oldMousePos, const glm::vec2& mouseDelta, const float& zoomScale, FBool ongoing);

		void edit(bool& cameraMoving);
		void update() const;

		void generateCode(std::ofstream& fs) const;

		std::vector<glm::vec2> getStartingLineEnds() const;
		float getStartingPositionLineDistance() const;

	private:
		struct GrappleData
		{
			std::list<ControlPoint>::iterator controlPoint;
			ComponentId grappleId{};
			float radius{};
			float range{};
		};

		const glm::vec2& mousePos;
		const glm::vec2& oldMousePos;
		const glm::vec2& mouseDelta;

		const float& zoomScale;

		FBool ongoing;

		std::list<ControlPoint> controlPoints;
		std::unordered_map<ComponentId, GrappleData> cpDecoIdsToGrapplesData;
		std::optional<std::list<ControlPoint>::iterator> movingControlPoint;
	};
}
