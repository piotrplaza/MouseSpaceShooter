#include <components/polyline.hpp>
#include <components/decoration.hpp>
#include <components/texture.hpp>

#include <globals/components.hpp>

#include <tools/Shapes2D.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <unordered_set>

namespace GeneratedCode
{

constexpr static glm::vec4 backgroundColor = {(float)0, (float)0.05, (float)0.2, (float)1};
constexpr static glm::vec2 backgroundImagePosition = {(float)0, (float)0};
constexpr static float backgroundImageAspectRatio = (float)1;
constexpr static glm::vec2 backgroundImageScale = {(float)200, (float)200};
constexpr static float projectionHSizeMin = (float)40;
constexpr static const char* musicPath = "audio/Ghosthack-Ambient Beds_Daylight_Am 75Bpm (WET).ogg";

inline void CreateBackground(ComponentId& backgroundTextureId, ComponentId& backgroundDecorationId)
{
	backgroundTextureId = Globals::Components().staticTextures().size();
	Globals::Components().staticTextures().emplace("textures/racing/space track 1.jpg");
	auto& background = Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({}, { 0.5f * backgroundImageAspectRatio, 0.5f }));
	background.texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
	background.modelMatrixF = glm::scale(glm::translate(glm::mat4{ 1.0f }, glm::vec3(backgroundImagePosition, 0.0f)), glm::vec3(backgroundImageScale, 1.0f));
	background.renderLayer = RenderLayer::Background;
	background.texture = CM::Texture(backgroundTextureId, true);
	backgroundDecorationId = background.getComponentId();
}

inline void CreateStartingLine(ComponentId& startingLineId, glm::vec2& p1, glm::vec2& p2, float& startingPositionLineDistance)
{
	const glm::vec2 cp1 = { 49.5001, 22.6 };
	const glm::vec2 cp2 = { 67.8, 22.8 };
	auto& startingLine = Globals::Components().staticPolylines().emplace(std::vector<glm::vec2>{ cp1, cp2 },
		Tools::BodyParams().sensor(true)); 
	startingLine.colorF = glm::vec4((float)0.5, (float)0.5, (float)0.5, (float)0.5);

	startingLineId = startingLine.getComponentId();
	p1 = cp1;
	p2 = cp2;
	startingPositionLineDistance = 2;
}

inline void CreateDeadlySplines(const Tools::PlayersHandler& playersHandler, std::unordered_set<ComponentId>& deadlySplines)
{
	auto& polylines = Globals::Components().staticPolylines();

	{
		std::vector<glm::vec2> controlPoints;
		controlPoints.reserve(37);
		controlPoints.push_back({68.2, 9.80013});
		controlPoints.push_back({70.8, -2.69988});
		controlPoints.push_back({78.1999, -14.2999});
		controlPoints.push_back({82.3998, -33.1999});
		controlPoints.push_back({76.9999, -50.6998});
		controlPoints.push_back({60.4999, -61.5998});
		controlPoints.push_back({39.8999, -61.5998});
		controlPoints.push_back({23.5999, -54.0999});
		controlPoints.push_back({11.3999, -40.9999});
		controlPoints.push_back({7.39991, -20.3999});
		controlPoints.push_back({12.8999, -2.7999});
		controlPoints.push_back({18.5999, 14.7001});
		controlPoints.push_back({9.69993, 27.9001});
		controlPoints.push_back({-5.30007, 29.1001});
		controlPoints.push_back({-15.6001, 20.1001});
		controlPoints.push_back({-16.2001, 6.50007});
		controlPoints.push_back({-7.60012, -15.2});
		controlPoints.push_back({-7.00012, -34});
		controlPoints.push_back({-16.0001, -53.1});
		controlPoints.push_back({-29.4001, -64});
		controlPoints.push_back({-47.7001, -68.1999});
		controlPoints.push_back({-62.4001, -64.3});
		controlPoints.push_back({-75.2001, -51.4});
		controlPoints.push_back({-77.6, -39.0999});
		controlPoints.push_back({-75.3, -26.4001});
		controlPoints.push_back({-68.7001, -10.4001});
		controlPoints.push_back({-70.1001, 4.09994});
		controlPoints.push_back({-71.3, 22.8999});
		controlPoints.push_back({-67.3, 41.3});
		controlPoints.push_back({-58.1, 58.2});
		controlPoints.push_back({-40.7, 74});
		controlPoints.push_back({-20.2999, 81.6998});
		controlPoints.push_back({9.40005, 83.0998});
		controlPoints.push_back({34.3, 75.2999});
		controlPoints.push_back({52.1, 60.0999});
		controlPoints.push_back({61.2, 46.3999});
		controlPoints.push_back({67.8, 23.5});

		const int numOfVertices = 10 * (controlPoints.size() - 1 + 1) + 1;
		Tools::CubicHermiteSpline spline(std::move(controlPoints), Tools::CubicHermiteSpline<>::loop);
		std::vector<glm::vec2> veritces;
		veritces.reserve(numOfVertices);
		for (int i = 0; i < numOfVertices; ++i)
			veritces.push_back(glm::vec3(spline.getSplineSample((float)i / (numOfVertices - 1)), 0.0f));
		polylines.emplace(std::move(veritces));
		polylines.last().colorF = glm::vec4((float)0.5, (float)0, (float)0, (float)0.5);
		polylines.last().segmentVerticesGenerator = [](const auto& v1, const auto& v2) { return Tools::Shapes2D::CreatePositionsOfLightning(v1, v2, std::max(1, (int)glm::distance(v1, v2)), 0.2f); };
		polylines.last().keyVerticesTransformer = [](std::vector<glm::vec3>& vertices) {
			Tools::VerticesDefaultRandomTranslate(vertices, 1, (float)0.05);
		};

		deadlySplines.insert(polylines.last().getComponentId());
	}

	{
		std::vector<glm::vec2> controlPoints;
		controlPoints.reserve(35);
		controlPoints.push_back({49.7005, 7.39986});
		controlPoints.push_back({53.9004, -9.50014});
		controlPoints.push_back({60.2004, -20.9001});
		controlPoints.push_back({62.9004, -34.0002});
		controlPoints.push_back({53.8004, -44.4001});
		controlPoints.push_back({38.2003, -43.3001});
		controlPoints.push_back({28.1003, -34.0002});
		controlPoints.push_back({25.9003, -20.3002});
		controlPoints.push_back({30.5003, -9.70015});
		controlPoints.push_back({35.5002, 1.49985});
		controlPoints.push_back({36.7002, 17.1999});
		controlPoints.push_back({30.7003, 33.0998});
		controlPoints.push_back({19.5002, 42.8997});
		controlPoints.push_back({5.60021, 47.7997});
		controlPoints.push_back({-12.5998, 45.1997});
		controlPoints.push_back({-25.5998, 36.3998});
		controlPoints.push_back({-34.0998, 21.2998});
		controlPoints.push_back({-34.1998, 2.09984});
		controlPoints.push_back({-27.9998, -12.9001});
		controlPoints.push_back({-25.2998, -29.7001});
		controlPoints.push_back({-32.9998, -44.6001});
		controlPoints.push_back({-50.4999, -49.9999});
		controlPoints.push_back({-58.4998, -43.0999});
		controlPoints.push_back({-57.4998, -30.3999});
		controlPoints.push_back({-50.9999, -15.8999});
		controlPoints.push_back({-49.7999, -2.49988});
		controlPoints.push_back({-52.7998, 14.5001});
		controlPoints.push_back({-50.3999, 33.3001});
		controlPoints.push_back({-41.1998, 51.1002});
		controlPoints.push_back({-26.4999, 61.4001});
		controlPoints.push_back({-4.39987, 66.4});
		controlPoints.push_back({17.9001, 63.2});
		controlPoints.push_back({32.1001, 54.6001});
		controlPoints.push_back({44.0001, 39.5001});
		controlPoints.push_back({49.2, 22.4001});

		const int numOfVertices = 10 * (controlPoints.size() - 1 + 1) + 1;
		Tools::CubicHermiteSpline spline(std::move(controlPoints), Tools::CubicHermiteSpline<>::loop);
		std::vector<glm::vec2> veritces;
		veritces.reserve(numOfVertices);
		for (int i = 0; i < numOfVertices; ++i)
			veritces.push_back(glm::vec3(spline.getSplineSample((float)i / (numOfVertices - 1)), 0.0f));
		polylines.emplace(std::move(veritces));
		polylines.last().colorF = glm::vec4((float)0.5, (float)0, (float)0, (float)0.5);
		polylines.last().segmentVerticesGenerator = [](const auto& v1, const auto& v2) { return Tools::Shapes2D::CreatePositionsOfLightning(v1, v2, std::max(1, (int)glm::distance(v1, v2)), 0.2f); };
		polylines.last().keyVerticesTransformer = [](std::vector<glm::vec3>& vertices) {
			Tools::VerticesDefaultRandomTranslate(vertices, 1, (float)0.05);
		};

		deadlySplines.insert(polylines.last().getComponentId());
	}
}

inline void CreateGrapples()
{
	Globals::Components().staticGrapples().emplace(Tools::CreateDiscBody((float)10.9752, Tools::BodyParams{}.position({ (float)0.3542, (float)13.692 }))).range = (float)75.901;
	Globals::Components().staticGrapples().last().colorF = glm::vec4((float)0, (float)0.5, (float)0.5, (float)0.5);
	Globals::Components().staticGrapples().emplace(Tools::CreateDiscBody((float)8.61727, Tools::BodyParams{}.position({ (float)-45.0299, (float)-34.2052 }))).range = (float)36.011;
	Globals::Components().staticGrapples().last().colorF = glm::vec4((float)0, (float)0.5, (float)0.5, (float)0.5);
	Globals::Components().staticGrapples().emplace(Tools::CreateDiscBody((float)11.2921, Tools::BodyParams{}.position({ (float)45.7291, (float)-28.691 }))).range = (float)40;
	Globals::Components().staticGrapples().last().colorF = glm::vec4((float)0, (float)0.5, (float)0.5, (float)0.5);
}

}
