#include "splinesEditing.hpp"

#include <components/keyboard.hpp>
#include <components/mouse.hpp>
#include <components/decoration.hpp>
#include <components/polyline.hpp>

#include <globals/components.hpp>

#include <tools/utility.hpp>
#include <tools/glmHelpers.hpp>
#include <tools/splines.hpp>
#include <tools/Shapes2D.hpp>
#include <tools/paramsFromFile.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <numeric>
#include <ranges>

namespace
{
	constexpr glm::vec4 activeControlPointColor(0.0f, 1.0f, 0.0f, 1.0f);
	constexpr glm::vec4 inactiveControlPointColor = activeControlPointColor * 0.5f;
	constexpr int controlPointComplexity = 20;
	constexpr float scalingFactor = 0.05f;
}

namespace Levels
{
	SplineEditing::SplineEditing(const glm::vec2& mousePos, const glm::vec2& oldMousePos, const glm::vec2& mouseDelta, const float& zoomScale, FBool ongoing, const Tools::ParamsFromFile& paramsFromFile):
		mousePos(mousePos),
		oldMousePos(oldMousePos),
		mouseDelta(mouseDelta),
		zoomScale(zoomScale),
		ongoing(std::move(ongoing)),
		paramsFromFile(paramsFromFile)
	{
		paramsFromFile.loadParam(controlPointRadius, "editor.controlPointRadius", false);
		paramsFromFile.loadParam(activeSplineColor, "splines.color", false);
		inactiveSplineColor = activeSplineColor * 0.5f;
	}

	void SplineEditing::edit()
	{
		const auto& mouse = Globals::Components().mouse();
		const auto& keyboard = Globals::Components().keyboard();
		auto& dynamicDecorations = Globals::Components().decorations();
		auto& polylines = Globals::Components().polylines();

		auto addControlPoint = [&](std::optional<std::list<ControlPoint>::iterator> it = std::nullopt) {
			if (!it)
			{
				if (!activePolylineId)
				{
					activePolylineId = polylines.emplace().getComponentId();
					polylines.last().colorF = [&, id = *activePolylineId]() {
						return (activePolylineId && id == *activePolylineId) || !ongoing()
							? activeSplineColor
							: inactiveSplineColor;
						};
					polylines.last().renderLayer = RenderLayer::FarMidground;
				}
			}

			auto& controlPoints = polylineIdToSplineDef[*activePolylineId].controlPoints;

			dynamicDecorations.emplace(Tools::Shapes2D::CreatePositionsOfDisc({ 0.0f, 0.0f }, controlPointRadius, controlPointComplexity));
			dynamicDecorations.last().colorF = [&, id = *activePolylineId]() {
				return (activePolylineId && id == *activePolylineId)
					? activeControlPointColor
					: inactiveControlPointColor;
			};

			auto insertedIt = controlPoints.insert(it ? *it : controlPoints.begin(), { dynamicDecorations.last().getComponentId(), mousePos });
			dynamicDecorations.last().modelMatrixF = [&, &pos = insertedIt->pos]() {
				return glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)), glm::vec3(zoomScale));
			};
			dynamicDecorations.last().renderF = ongoing;
			dynamicDecorations.last().renderLayer = RenderLayer::NearMidground;

			return insertedIt;
		};

		auto controlPointAction = [&](std::function<void(ComponentId, SplineDef&, std::list<ControlPoint>::iterator)> action)
		{
			for (auto& [splineId, splineDef] : polylineIdToSplineDef)
				for (auto it = splineDef.controlPoints.begin(); it != splineDef.controlPoints.end(); ++it)
					if (glm::distance(it->pos, oldMousePos) < controlPointRadius * zoomScale)
					{
						activePolylineId = splineId;
						action(splineId, splineDef, it);

						return;
					}
		};

		auto addOrMoveControlPoint = [&]() {
			if (movingAdjacentControlPoint)
				return;

			if (!movingControlPoint)
				controlPointAction([&](auto, auto, auto it) {
					movingControlPoint = it;
				});

			if (movingControlPoint)
			{
				if (keyboard.pressing[/*VK_SHIFT*/0x10] && activePolylineId)
				{
					auto& splineDef = polylineIdToSplineDef[*activePolylineId];
					for (auto& controlPoint : splineDef.controlPoints)
						controlPoint.pos += mouseDelta;
				}
				else
					(*movingControlPoint)->pos += mouseDelta;

				return;
			}

			if (activePolylineId)
			{
				auto& controlPoints = polylineIdToSplineDef[*activePolylineId].controlPoints;

				addControlPoint(keyboard.pressing[/*VK_SHIFT*/0x10]
					? controlPoints.end()
					: controlPoints.begin());
			}
			else
				addControlPoint();
		};

		auto removeControlPoint = [&]() {
			controlPointAction([&](auto splineId, auto& splineDef, auto it) {
				if (movingControlPoint || movingAdjacentControlPoint)
					return;

				dynamicDecorations[it->decorationId].state = ComponentState::Outdated;
				splineDef.controlPoints.erase(it);
				if (splineDef.controlPoints.empty())
					polylineIdToSplineDef.erase(splineId);
			});
		};

		auto removeActiveSpline = [&]() {
			auto& dynamicDecorations = Globals::Components().decorations();
			auto& polylines = Globals::Components().polylines();

			for (const auto& controlPoint : polylineIdToSplineDef[*activePolylineId].controlPoints)
				dynamicDecorations[controlPoint.decorationId].state = ComponentState::Outdated;

			polylines[*activePolylineId].state = ComponentState::Outdated;
			polylineIdToSplineDef.erase(*activePolylineId);
			activePolylineId = std::nullopt;
		};

		auto tryAddOrMoveAdjacentControlPoint = [&]() {
			if (!movingAdjacentControlPoint)
				controlPointAction([&](auto, auto, auto it) {
					movingAdjacentControlPoint = addControlPoint(keyboard.pressing[/*VK_SHIFT*/0x10]
					? std::next(it) : it);
				});

			if (movingAdjacentControlPoint)
				(*movingAdjacentControlPoint)->pos += mouseDelta;
		};

		auto deactivate = [&]() {
			auto& polyline = Globals::Components().polylines()[*activePolylineId];
			activePolylineId = std::nullopt;
		};

		auto copy = [&]() {
			auto& sourcePolyline = Globals::Components().polylines()[*activePolylineId];
			auto& targetPolyline = Globals::Components().polylines().emplace();
			targetPolyline.colorF = [&, id = targetPolyline.getComponentId()]() {
				return (activePolylineId && id == *activePolylineId) || !ongoing()
					? activeSplineColor
					: inactiveSplineColor;
			};
			targetPolyline.drawMode = GL_LINE_STRIP;
			targetPolyline.renderLayer = RenderLayer::FarMidground;
			polylineIdToSplineDef[targetPolyline.getComponentId()] =
				polylineIdToSplineDef[sourcePolyline.getComponentId()];
			for (auto& controlPoint : polylineIdToSplineDef[targetPolyline.getComponentId()].controlPoints)
			{
				auto& controlPointDecoration = dynamicDecorations.emplace(Tools::Shapes2D::CreatePositionsOfDisc({ 0.0f, 0.0f }, controlPointRadius,
					controlPointComplexity));
				controlPoint.decorationId = controlPointDecoration.getComponentId();
				controlPointDecoration.colorF = [&, id = targetPolyline.getComponentId()]() {
					return (activePolylineId && id == *activePolylineId)
						? activeControlPointColor
						: inactiveControlPointColor;
				};
				controlPointDecoration.modelMatrixF = [&, &pos = controlPoint.pos]() {
					return glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)), glm::vec3(zoomScale));
				};
				controlPointDecoration.renderF = ongoing;
				controlPointDecoration.renderLayer = RenderLayer::NearMidground;
			}
			
		};

		auto scaleActiveSpline = [&]() {
			auto& controlPoints = polylineIdToSplineDef[*activePolylineId].controlPoints;
			const auto center = std::accumulate(controlPoints.begin(), controlPoints.end(), glm::vec2(0.0f), [](auto sum, const auto& cp) {
				return sum + cp.pos; }) / (float)controlPoints.size();

			for (auto& cp : controlPoints)
			{
				auto d = cp.pos - center;
				cp.pos = center + d * (1.0f + mouse.pressed.wheel * scalingFactor);
			}
		};

		if (mouse.pressing.lmb)
			addOrMoveControlPoint();
		else
			movingControlPoint = std::nullopt;

		if (mouse.pressing.rmb && keyboard.pressing[/*VK_SHIFT*/0x10])
			removeControlPoint();

		if (mouse.pressing.mmb)
			tryAddOrMoveAdjacentControlPoint();
		else
			movingAdjacentControlPoint = std::nullopt;

		if (activePolylineId)
		{
			auto& splineDef = polylineIdToSplineDef[*activePolylineId];

			if (keyboard.pressed['L'])
				splineDef.loop = !splineDef.loop;

			if (keyboard.pressed['D'])
				deactivate();

			if (keyboard.pressed['C'])
				copy();

			if (mouse.pressed.wheel && keyboard.pressing[/*VK_SHIFT*/0x10])
				splineDef.complexity = std::clamp(splineDef.complexity + mouse.pressed.wheel, 1, 50);

			if (mouse.pressed.wheel && keyboard.pressing[/*VK_CONTROL*/0x11])
				scaleActiveSpline();

			if (keyboard.pressed[/*VK_DELETE*/0x2E] || (mouse.pressing.rmb && keyboard.pressing[/*VK_CONTROL*/0x11]))
				removeActiveSpline();

			if (keyboard.pressing[/*VK_MENU*/0x12] && mouse.pressed.wheel)
				splineDef.lightning = std::clamp(splineDef.lightning + mouse.pressed.wheel, 0, 50);
		}
	}

	void SplineEditing::update() const
	{
		for (auto& [polylineId, splineDef] : polylineIdToSplineDef)
		{
			const auto& splineDef = polylineIdToSplineDef.at(polylineId);
			auto& polyline = Globals::Components().polylines()[polylineId];

			if (splineDef.controlPoints.size() < 2)
			{
				polyline.replaceFixtures({});
				polyline.state = ComponentState::Changed;
				continue;
			}

			std::vector<glm::vec2> controlPointsPos;
			controlPointsPos.reserve(splineDef.controlPoints.size());
			for (const auto& controlPoint : splineDef.controlPoints)
				controlPointsPos.push_back(controlPoint.pos);

			const int numOfVertices = ((int)controlPointsPos.size() - !splineDef.loop) * splineDef.complexity + 1;
			std::vector<glm::vec2> veritces;
			veritces.reserve(numOfVertices);
			Tools::CubicHermiteSpline spline = splineDef.loop
				? Tools::CubicHermiteSpline(std::move(controlPointsPos), Tools::CubicHermiteSpline<>::loop)
				: Tools::CubicHermiteSpline(std::move(controlPointsPos));
			for (int i = 0; i < numOfVertices; ++i)
				veritces.push_back(glm::vec3(spline.getSplineSample((float)i / (numOfVertices - 1)), 0.0f));

			polyline.replaceFixtures(veritces);
			if (splineDef.lightning)
			{
				polyline.segmentVerticesGenerator = [&](const auto& v1, const auto& v2) {
					return Tools::Shapes2D::CreatePositionsOfLightning(v1, v2, std::max(1, (int)glm::distance(v1, v2)), sqrt(glm::distance(v1, v2)) * splineDef.complexity * splineDef.lightning * 0.001f);
				};
				polyline.keyVerticesTransformer = [&](std::vector<glm::vec3>& vertices) {
					Tools::VerticesDefaultRandomTranslate(vertices, splineDef.loop, (float)(splineDef.complexity * splineDef.lightning * 0.001f));
				};
			}
			else
			{
				polyline.segmentVerticesGenerator = {};
				polyline.keyVerticesTransformer = {};
			}
		}
	}

	void SplineEditing::generateCode(std::ofstream& fs) const
	{
		fs << "inline void CreateDeadlySplines(const Tools::PlayersHandler& playersHandler, std::unordered_set<ComponentId>& deadlySplines)\n";
		fs << "{\n";
		if (!polylineIdToSplineDef.empty())
			fs << "	auto& polylines = Globals::Components().staticPolylines();\n";

		auto createControlPoints = [&](const auto& splineDef) {
			fs << "		std::vector<glm::vec2> controlPoints;\n";
			fs << "		controlPoints.reserve(" << splineDef.controlPoints.size() << ");\n";
			for (const auto& cp : splineDef.controlPoints)
				fs << "		controlPoints.push_back({" << cp.pos.x << ", " << cp.pos.y << "});\n";
		};

		for (const auto& [splineDecorationId, splineDef] : polylineIdToSplineDef)
		{
			if (splineDef.controlPoints.size() < 2)
				continue;

			fs << "\n	{\n";
			createControlPoints(splineDef);
			fs << "\n		const int numOfVertices = " << splineDef.complexity << " * (controlPoints.size() - 1 + " << splineDef.loop << ") + 1;\n";
			fs << "		Tools::CubicHermiteSpline spline(std::move(controlPoints)" << (splineDef.loop ? ", Tools::CubicHermiteSpline<>::loop);\n" : ");\n");
			fs << "		std::vector<glm::vec2> veritces;\n";
			fs << "		veritces.reserve(numOfVertices);\n";
			fs << "		for (int i = 0; i < numOfVertices; ++i)\n";
			fs << "			veritces.push_back(glm::vec3(spline.getSplineSample((float)i / (numOfVertices - 1)), 0.0f));\n";

			fs << "		polylines.emplace(std::move(veritces));\n";
			fs << "		polylines.last().colorF = glm::vec4((float)" << activeSplineColor.r << ", (float)" << activeSplineColor.g << ", (float)" << activeSplineColor.b << ", (float)" << activeSplineColor.a << ");\n";

			if (splineDef.lightning)
			{
				fs << "		polylines.last().segmentVerticesGenerator = [](const auto& v1, const auto& v2) { " <<
					"return Tools::Shapes2D::CreatePositionsOfLightning(v1, v2, std::max(1, (int)glm::distance(v1, v2)), 0.2f); };\n";
				fs << "		polylines.last().keyVerticesTransformer = [](std::vector<glm::vec3>& vertices) {\n";
				fs << "			Tools::VerticesDefaultRandomTranslate(vertices, " << splineDef.loop << ", (float)" << (splineDef.complexity * 0.005f) << ");\n";
				fs << "		};\n";
			}

			fs << "\n";
			fs << "		deadlySplines.insert(polylines.last().getComponentId());\n";
			fs << "	}\n";
		}
		fs << "}\n";
		fs << "\n";
	}
}
