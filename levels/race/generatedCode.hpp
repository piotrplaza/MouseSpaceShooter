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
	const glm::vec2 cp1 = { 14.4, -1.35 };
	const glm::vec2 cp2 = { 49.25, -15.35 };
	auto& startingLine = Globals::Components().staticPolylines().emplace(std::vector<glm::vec2>{ cp1, cp2 },
		Tools::BodyParams().sensor(true)); 
	startingLine.colorF = []() { return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); };

	startingLineId = startingLine.getComponentId();
	p1 = cp1;
	p2 = cp2;
	startingPositionLineDistance = 1;
}

inline void CreateDeadlySplines(const Tools::PlayersHandler& playersHandler, std::unordered_set<ComponentId>& deadlySplines)
{
	auto& polylines = Globals::Components().staticPolylines();

	{
		std::vector<glm::vec2> controlPoints;
		controlPoints.reserve(8);
		controlPoints.push_back({-175.345, -363.185});
		controlPoints.push_back({-286.634, -44.0793});
		controlPoints.push_back({-406.252, -145.906});
		controlPoints.push_back({-519.812, 128.155});
		controlPoints.push_back({-298.368, 360.576});
		controlPoints.push_back({-65.19, 231.873});
		controlPoints.push_back({114.993, 259.885});
		controlPoints.push_back({54.4276, -16.0682});

		const int numOfVertices = 18 * (controlPoints.size() - 1 + 1) + 1;
		Tools::CubicHermiteSpline spline(std::move(controlPoints), Tools::CubicHermiteSpline<>::loop);
		std::vector<glm::vec2> veritces;
		veritces.reserve(numOfVertices);
		for (int i = 0; i < numOfVertices; ++i)
			veritces.push_back(glm::vec3(spline.getSplineSample((float)i / (numOfVertices - 1)), 0.0f));
		polylines.emplace(std::move(veritces));
		polylines.last().segmentVerticesGenerator = [](const auto& v1, const auto& v2) { return Tools::Shapes2D::CreatePositionsOfLightning(v1, v2, std::max(1, (int)glm::distance(v1, v2)), 0.2f); };
		polylines.last().keyVerticesTransformer = [](std::vector<glm::vec3>& vertices) {
			Tools::VerticesDefaultRandomTranslate(vertices, 1, (float)0.09);
		};

		deadlySplines.insert(polylines.last().getComponentId());
	}

	{
		std::vector<glm::vec2> controlPoints;
		controlPoints.reserve(7);
		controlPoints.push_back({-165.552, -220.738});
		controlPoints.push_back({-241.639, 36.1467});
		controlPoints.push_back({-369.9, 25.6393});
		controlPoints.push_back({-298.523, 277.814});
		controlPoints.push_back({-85.8413, 150.278});
		controlPoints.push_back({41.6954, 205.35});
		controlPoints.push_back({10.1735, 1.72643});

		const int numOfVertices = 17 * (controlPoints.size() - 1 + 1) + 1;
		Tools::CubicHermiteSpline spline(std::move(controlPoints), Tools::CubicHermiteSpline<>::loop);
		std::vector<glm::vec2> veritces;
		veritces.reserve(numOfVertices);
		for (int i = 0; i < numOfVertices; ++i)
			veritces.push_back(glm::vec3(spline.getSplineSample((float)i / (numOfVertices - 1)), 0.0f));
		polylines.emplace(std::move(veritces));
		polylines.last().segmentVerticesGenerator = [](const auto& v1, const auto& v2) { return Tools::Shapes2D::CreatePositionsOfLightning(v1, v2, std::max(1, (int)glm::distance(v1, v2)), 0.2f); };
		polylines.last().keyVerticesTransformer = [](std::vector<glm::vec3>& vertices) {
			Tools::VerticesDefaultRandomTranslate(vertices, 1, (float)0.085);
		};

		deadlySplines.insert(polylines.last().getComponentId());
	}
}

inline void CreateGrapples()
{
	Globals::Components().staticGrapples().emplace(Tools::CreateDiscBody((float)2, Tools::BodyParams{}.position({ (float)-186.908, (float)-210.597 }))).range = (float)40; 
	Globals::Components().staticGrapples().emplace(Tools::CreateDiscBody((float)2, Tools::BodyParams{}.position({ (float)59.4006, (float)186.913 }))).range = (float)40; 
	Globals::Components().staticGrapples().emplace(Tools::CreateDiscBody((float)2, Tools::BodyParams{}.position({ (float)-258.235, (float)279.417 }))).range = (float)40; 
	Globals::Components().staticGrapples().emplace(Tools::CreateDiscBody((float)2, Tools::BodyParams{}.position({ (float)-382.318, (float)53.5423 }))).range = (float)40; 
	Globals::Components().staticGrapples().emplace(Tools::CreateDiscBody((float)2, Tools::BodyParams{}.position({ (float)-282.384, (float)-39.7052 }))).range = (float)40; 
}

}
