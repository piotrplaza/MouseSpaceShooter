#pragma once

#include "common.hpp"

#include <commonTypes/fTypes.hpp>

#include <glm/vec2.hpp>

#include <fstream>
#include <optional>
#include <list>
#include <unordered_map>

namespace Tools
{
	class ParamsFromFile;
}

namespace Levels
{
	class GrappleEditing
	{
	public:
		GrappleEditing(const glm::vec2& mousePos, const glm::vec2& oldMousePos, const glm::vec2& mouseDelta, const float& zoomScale, FBool ongoing, const Tools::ParamsFromFile& paramsFromFile);

		void edit();
		void update() const;

		void generateCode(std::ofstream& fs) const;

		std::vector<glm::vec2> getStartingLineEnds() const;
		float getStartingPositionLineDistance() const;

	private:
		struct GrappleData
		{
			std::list<ControlPoint>::iterator controlPoint;
			ComponentId grappleId{};
			ComponentId rangeDecorationId{};
			float radius{};
			float range{};
		};

		const glm::vec2& mousePos;
		const glm::vec2& oldMousePos;
		const glm::vec2& mouseDelta;

		const float& zoomScale;

		FBool ongoing;

		const Tools::ParamsFromFile& paramsFromFile;

		std::list<ControlPoint> controlPoints;
		std::unordered_map<ComponentId, GrappleData> cpDecoIdsToGrapplesData;
		std::optional<std::list<ControlPoint>::iterator> movingControlPoint;

		float controlPointRadius = 0.75f;
		float grappleInitRadius = 2.0f;
		float grappleInitRange = 40.0f;
	};
}
