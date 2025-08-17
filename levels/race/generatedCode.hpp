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

constexpr static float scale = (float)1.5;
constexpr static glm::vec4 backgroundColor = {(float)0, (float)0.05, (float)0.2, (float)1};
constexpr static glm::vec2 backgroundImagePosition = {(float)0, (float)0};
constexpr static float backgroundImageAspectRatio = (float)1;
constexpr static glm::vec2 backgroundImageScale = {(float)200, (float)200};
constexpr static float projectionHSizeMin = (float)30;
constexpr static const char* musicPath = "audio/Ghosthack-Ambient Beds_Daylight_Am 75Bpm (WET).ogg";

inline void CreateBackground(ComponentId& backgroundTextureId, ComponentId& backgroundDecorationId)
{
	backgroundTextureId = Globals::Components().staticTextures().size();
	Globals::Components().staticTextures().emplace("textures/racing/space track 1.jpg");
	auto& background = Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({}, { 0.5f * backgroundImageAspectRatio, 0.5f }));
	background.texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
	background.modelMatrixF = glm::scale(glm::translate(glm::mat4{ 1.0f }, glm::vec3(backgroundImagePosition, 0.0f)), glm::vec3(backgroundImageScale * scale, 1.0f));
	background.renderLayer = RenderLayer::Background;
	background.texture = CM::Texture(backgroundTextureId, true);
	backgroundDecorationId = background.getComponentId();
}

inline void CreateStartingLine(ComponentId& startingLineId, glm::vec2& p1, glm::vec2& p2, float& startingPositionLineDistance)
{
	const glm::vec2 cp1 = { 49.5552, 18.5712 };
	const glm::vec2 cp2 = { 68.2357, 19.3 };
	auto& startingLine = Globals::Components().staticPolylines().emplace(std::vector<glm::vec2>{ cp1 * scale, cp2 * scale },
		Tools::BodyParams().sensor(true)); 
	startingLine.colorF = glm::vec4((float)0.5, (float)0.5, (float)0.5, (float)0.5);

	startingLineId = startingLine.getComponentId();
	p1 = cp1 * scale;
	p2 = cp2 * scale;
	startingPositionLineDistance = 10.0f;
}

inline void CreateDeadlySplines(const Tools::PlayersHandler& playersHandler, std::unordered_set<ComponentId>& deadlySplines)
{
	auto& polylines = Globals::Components().staticPolylines();

	{
		std::vector<glm::vec2> keypoints;
		keypoints.reserve(42);
		keypoints.push_back({68.5125, 6.8672});
		keypoints.push_back({72.0207, -4.81604});
		keypoints.push_back({77.9131, -14.6127});
		keypoints.push_back({81.8776, -26.5475});
		keypoints.push_back({80.9809, -40.2683});
		keypoints.push_back({75.0183, -52.3077});
		keypoints.push_back({64.3479, -60.2383});
		keypoints.push_back({51.5982, -62.5865});
		keypoints.push_back({34.4378, -60.2666});
		keypoints.push_back({18.2684, -50.1181});
		keypoints.push_back({9.22241, -35.5511});
		keypoints.push_back({7.617, -18.172});
		keypoints.push_back({10.581, -7.36794});
		keypoints.push_back({18.4973, 11.6451});
		keypoints.push_back({13.9934, 24.3476});
		keypoints.push_back({0.996064, 30.169});
		keypoints.push_back({-12.4628, 24.3012});
		keypoints.push_back({-17.1145, 12.0338});
		keypoints.push_back({-13.3986, -0.836329});
		keypoints.push_back({-7.32478, -15.1558});
		keypoints.push_back({-7.33087, -33.9032});
		keypoints.push_back({-14.2934, -50.4781});
		keypoints.push_back({-27.5057, -62.9774});
		keypoints.push_back({-44.67, -67.9459});
		keypoints.push_back({-61.1941, -65.2402});
		keypoints.push_back({-73.4853, -54.287});
		keypoints.push_back({-77.6178, -40.9823});
		keypoints.push_back({-75.3502, -26.6159});
		keypoints.push_back({-69.9878, -15.3464});
		keypoints.push_back({-68.4497, -6.07422});
		keypoints.push_back({-70.6737, 6.34102});
		keypoints.push_back({-71.4603, 18.162});
		keypoints.push_back({-69.784, 32.3745});
		keypoints.push_back({-64.1155, 49.1095});
		keypoints.push_back({-50.223, 67.3501});
		keypoints.push_back({-29.5555, 79.2788});
		keypoints.push_back({-4.02468, 83.8272});
		keypoints.push_back({23.8949, 79.7012});
		keypoints.push_back({45.2804, 67.6789});
		keypoints.push_back({58.6384, 50.6081});
		keypoints.push_back({65.405, 34.3855});
		keypoints.push_back({68.1938, 19.3509});
		const int numOfVertices = 10 * ((int)keypoints.size() - 1 + 1) + 1;
		Tools::CubicHermiteSpline spline(std::move(keypoints), Tools::CubicHermiteSpline<>::loop);
		std::vector<glm::vec2> veritces;
		veritces.reserve(numOfVertices);
		for (int i = 0; i < numOfVertices; ++i)
			veritces.push_back(glm::vec3(spline.getSplineSample((float)i / (numOfVertices - 1)) * scale, 0.0f));
		polylines.emplace(std::move(veritces));
		polylines.last().colorF = glm::vec4((float)0.5, (float)0, (float)0, (float)0.5);
		polylines.last().segmentVerticesGenerator = [](const auto& v1, const auto& v2) { return Tools::Shapes2D::CreatePositionsOfLightning(v1, v2, std::max(1, (int)glm::distance(v1, v2)), 0.2f); };
		polylines.last().keyVerticesTransformer = [](std::vector<glm::vec3>& vertices) {
			Tools::VerticesDefaultRandomTranslate(vertices, 1, (float)0.05);
		};

		deadlySplines.insert(polylines.last().getComponentId());
	}

	{
		std::vector<glm::vec2> keypoints;
		keypoints.reserve(45);
		keypoints.push_back({49.6428, 9.60576});
		keypoints.push_back({50.811, 0.524964});
		keypoints.push_back({54.1491, -10.0097});
		keypoints.push_back({60.5103, -21.6088});
		keypoints.push_back({63.0793, -32.625});
		keypoints.push_back({58.4174, -41.7829});
		keypoints.push_back({47.755, -45.2388});
		keypoints.push_back({36.1028, -41.9669});
		keypoints.push_back({27.3747, -32.2178});
		keypoints.push_back({25.8967, -21.4767});
		keypoints.push_back({28.9714, -12.0654});
		keypoints.push_back({34.7938, -0.0808939});
		keypoints.push_back({36.9021, 12.3653});
		keypoints.push_back({34.8789, 25.6256});
		keypoints.push_back({27.3703, 37.2867});
		keypoints.push_back({15.2349, 44.9949});
		keypoints.push_back({0.272372, 47.8928});
		keypoints.push_back({-14.268, 44.6696});
		keypoints.push_back({-24.8842, 37.2914});
		keypoints.push_back({-31.4469, 28.8438});
		keypoints.push_back({-34.9583, 18.2267});
		keypoints.push_back({-34.8969, 5.45896});
		keypoints.push_back({-30.8183, -6.62988});
		keypoints.push_back({-25.6776, -20.0302});
		keypoints.push_back({-25.8357, -31.5331});
		keypoints.push_back({-31.3775, -42.661});
		keypoints.push_back({-41.2125, -48.8073});
		keypoints.push_back({-50.8971, -49.2816});
		keypoints.push_back({-57.911, -44.0747});
		keypoints.push_back({-58.8029, -34.6954});
		keypoints.push_back({-54.3814, -24.2993});
		keypoints.push_back({-50.3438, -12.809});
		keypoints.push_back({-50.3961, -0.163058});
		keypoints.push_back({-52.7155, 13.3987});
		keypoints.push_back({-51.5692, 27.8268});
		keypoints.push_back({-47.0233, 41.069});
		keypoints.push_back({-38.7998, 52.6367});
		keypoints.push_back({-27.6937, 60.8335});
		keypoints.push_back({-14.8467, 65.1025});
		keypoints.push_back({2.98083, 66.407});
		keypoints.push_back({18.954, 62.4041});
		keypoints.push_back({31.3231, 55.5365});
		keypoints.push_back({40.4829, 44.948});
		keypoints.push_back({46.5812, 32.9655});
		keypoints.push_back({49.5458, 18.6165});
		const int numOfVertices = 10 * ((int)keypoints.size() - 1 + 1) + 1;
		Tools::CubicHermiteSpline spline(std::move(keypoints), Tools::CubicHermiteSpline<>::loop);
		std::vector<glm::vec2> veritces;
		veritces.reserve(numOfVertices);
		for (int i = 0; i < numOfVertices; ++i)
			veritces.push_back(glm::vec3(spline.getSplineSample((float)i / (numOfVertices - 1)) * scale, 0.0f));
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
	Globals::Components().staticGrapples().emplace(Tools::CreateDiscBody((float)8.88655 * scale, Tools::BodyParams{}.position(glm::vec2((float)-44.7719, (float)-34.8457 ) * scale))).range = (float)35.6727 * scale;
	Globals::Components().staticGrapples().last().colorF = glm::vec4((float)0, (float)0.5, (float)0.5, (float)0.5);
	Globals::Components().staticGrapples().emplace(Tools::CreateDiscBody((float)11.7363 * scale, Tools::BodyParams{}.position(glm::vec2((float)0.771401, (float)13.2655 ) * scale))).range = (float)74.618 * scale;
	Globals::Components().staticGrapples().last().colorF = glm::vec4((float)0, (float)0.5, (float)0.5, (float)0.5);
	Globals::Components().staticGrapples().emplace(Tools::CreateDiscBody((float)11.7363 * scale, Tools::BodyParams{}.position(glm::vec2((float)44.5848, (float)-26.3289 ) * scale))).range = (float)40 * scale;
	Globals::Components().staticGrapples().last().colorF = glm::vec4((float)0, (float)0.5, (float)0.5, (float)0.5);
}

}
