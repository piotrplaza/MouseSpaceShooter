#include <components/polyline.hpp>

#include <globals/components.hpp>

#include <tools/Shapes2D.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <unordered_set>

namespace GeneratedCode
{

inline void CreateStartingLine(ComponentId& startingLineId, glm::vec2& p1, glm::vec2& p2, float& startingPositionLineDistance)
{
	const glm::vec2 cp1 = { 21.4867, 40.0026 };
	const glm::vec2 cp2 = { 14.6085, 26.9677 };
	auto& startingLine = Globals::Components().staticPolylines().emplace(std::vector<glm::vec2>{ cp1, cp2 },
		Tools::BodyParams().sensor(true)); 
	startingLine.colorF = []() { return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); };

	startingLineId = startingLine.getComponentId();
	p1 = cp1;
	p2 = cp2;
	startingPositionLineDistance = 6;
}

inline void CreateDeadlySplines(const Tools::PlayersHandler& playersHandler, std::unordered_set<ComponentId>& deadlySplines)
{
	auto& polylines = Globals::Components().staticPolylines();

	{
		std::vector<glm::vec2> controlPoints;
		controlPoints.reserve(9);
		controlPoints.push_back({-32.2, 31.65});
		controlPoints.push_back({17.5, 18.3});
		controlPoints.push_back({-13.95, 17.1});
		controlPoints.push_back({-15.8, 4.3});
		controlPoints.push_back({-43.6, 18.65});
		controlPoints.push_back({-32.1, -5.05});
		controlPoints.push_back({25.1719, -19.8901});
		controlPoints.push_back({40.796, 6.37242});
		controlPoints.push_back({-0.60001, 34.05});

		const int numOfVertices = 10 * ((int)controlPoints.size() - 1 + 1) + 1;
		Tools::CubicHermiteSpline spline(std::move(controlPoints), Tools::CubicHermiteSpline<>::loop);
		std::vector<glm::vec2> veritces;
		veritces.reserve(numOfVertices);
		for (int i = 0; i < numOfVertices; ++i)
			veritces.push_back(glm::vec3(spline.getSplineSample((float)i / (numOfVertices - 1)), 0.0f));
		polylines.emplace(std::move(veritces), Tools::BodyParams().sensor(true));

		deadlySplines.insert(polylines.last().getComponentId());
	}

	{
		std::vector<glm::vec2> controlPoints;
		controlPoints.reserve(8);
		controlPoints.push_back({-57.1002, 37.6});
		controlPoints.push_back({-28.6001, 22.55});
		controlPoints.push_back({-62.0001, 25.4999});
		controlPoints.push_back({-42.05, -24.2001});
		controlPoints.push_back({55.3697, -46.8563});
		controlPoints.push_back({52.1, 15.65});
		controlPoints.push_back({5.84152, 47.3518});
		controlPoints.push_back({-24.9221, 49.9903});

		const int numOfVertices = 10 * ((int)controlPoints.size() - 1 + 1) + 1;
		Tools::CubicHermiteSpline spline(std::move(controlPoints), Tools::CubicHermiteSpline<>::loop);
		std::vector<glm::vec2> veritces;
		veritces.reserve(numOfVertices);
		for (int i = 0; i < numOfVertices; ++i)
			veritces.push_back(glm::vec3(spline.getSplineSample((float)i / (numOfVertices - 1)), 0.0f));
		polylines.emplace(std::move(veritces), Tools::BodyParams().sensor(true));

		deadlySplines.insert(polylines.last().getComponentId());
	}
}

}
