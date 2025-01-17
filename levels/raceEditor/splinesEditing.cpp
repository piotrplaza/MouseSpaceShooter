#include "splinesEditing.hpp"

#include <components/keyboard.hpp>
#include <components/mouse.hpp>
#include <components/decoration.hpp>

#include <globals/components.hpp>

#include <tools/utility.hpp>
#include <tools/glmHelpers.hpp>
#include <tools/splines.hpp>
#include <tools/Shapes2D.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <numeric>

namespace
{
	constexpr glm::vec4 activeSplineColor(0.0f, 0.0f, 1.0f, 1.0f);
	constexpr glm::vec4 inactiveSplineColor = activeSplineColor * 0.5f;
	constexpr glm::vec4 activeControlPointColor(0.0f, 1.0f, 0.0f, 1.0f);
	constexpr glm::vec4 inactiveControlPointColor = activeControlPointColor * 0.5f;
	constexpr float controlPointRadius = 0.75f;
	constexpr int controlPointComplexity = 20;
	constexpr float scalingFactor = 0.05f;
}

namespace Levels
{
	SplineEditing::SplineEditing(const glm::vec2& mousePos, const glm::vec2& oldMousePos, const glm::vec2& mouseDelta, const float& zoomScale, FBool ongoing):
		mousePos(mousePos),
		oldMousePos(oldMousePos),
		mouseDelta(mouseDelta),
		zoomScale(zoomScale),
		ongoing(std::move(ongoing))
	{
	}

	void SplineEditing::edit(bool& cameraMoving)
	{
		const auto& mouse = Globals::Components().mouse();
		const auto& keyboard = Globals::Components().keyboard();
		auto& dynamicDecorations = Globals::Components().decorations();

		auto addControlPoint = [&](std::optional<std::list<ControlPoint>::iterator> it = std::nullopt) {
			if (!it)
			{
				if (!activeSplineDecorationId)
					activeSplineDecorationId = dynamicDecorations.emplace().getComponentId();

				dynamicDecorations.last().colorF = [&, id = *activeSplineDecorationId]() {
					return (activeSplineDecorationId && id == *activeSplineDecorationId) || !ongoing()
						? activeSplineColor
						: inactiveSplineColor;
				};
				dynamicDecorations.last().drawMode = GL_LINE_STRIP;
				dynamicDecorations.last().renderLayer = RenderLayer::FarMidground;
			}

			auto& controlPoints = splineDecorationIdToSplineDef[*activeSplineDecorationId].controlPoints;

			dynamicDecorations.emplace(Tools::Shapes2D::CreateVerticesOfCircle({ 0.0f, 0.0f }, controlPointRadius, controlPointComplexity));
			dynamicDecorations.last().colorF = [&, id = *activeSplineDecorationId]() {
				return (activeSplineDecorationId && id == *activeSplineDecorationId)
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
			for (auto& [splineDecorationId, splineDef] : splineDecorationIdToSplineDef)
				for (auto it = splineDef.controlPoints.begin(); it != splineDef.controlPoints.end(); ++it)
					if (glm::distance(it->pos, oldMousePos) < controlPointRadius * zoomScale)
					{
						activeSplineDecorationId = splineDecorationId;
						action(splineDecorationId, splineDef, it);

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
				if (keyboard.pressing[/*VK_SHIFT*/0x10] && activeSplineDecorationId)
				{
					auto& splineDef = splineDecorationIdToSplineDef[*activeSplineDecorationId];
					for (auto& controlPoint : splineDef.controlPoints)
						controlPoint.pos += mouseDelta;
				}
				else
					(*movingControlPoint)->pos += mouseDelta;

				return;
			}

			if (activeSplineDecorationId)
			{
				auto& controlPoints = splineDecorationIdToSplineDef[*activeSplineDecorationId].controlPoints;

				addControlPoint(keyboard.pressing[/*VK_SHIFT*/0x10]
					? controlPoints.end()
					: controlPoints.begin());
			}
			else
				addControlPoint();
		};

		auto removeControlPoint = [&]() {
			if (cameraMoving)
				return;

			bool removed = false;
			controlPointAction([&](auto splineDecorationId, auto& splineDef, auto it) {
				if (movingControlPoint || movingAdjacentControlPoint)
					return;

				if (keyboard.pressing[/*VK_SHIFT*/0x10])
				{
					for (const auto& controlPoint : splineDef.controlPoints)
						dynamicDecorations[controlPoint.decorationId].state = ComponentState::Outdated;
					splineDecorationIdToSplineDef.erase(splineDecorationId);
				}
				else
				{
					dynamicDecorations[it->decorationId].state = ComponentState::Outdated;
					splineDef.controlPoints.erase(it);
					if (splineDef.controlPoints.empty())
						splineDecorationIdToSplineDef.erase(splineDecorationId);
				}

				removed = true;
			});

			if (!removed)
				cameraMoving = true;
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
			auto& splineDecoration = Globals::Components().decorations()[*activeSplineDecorationId];
			activeSplineDecorationId = std::nullopt;
		};

		auto copy = [&]() {
			auto& sourceSplineDecoration = Globals::Components().decorations()[*activeSplineDecorationId];
			auto& targetSplineDecoration = Globals::Components().decorations().emplace();
			targetSplineDecoration.colorF = [&, id = targetSplineDecoration.getComponentId()]() {
				return (activeSplineDecorationId && id == *activeSplineDecorationId) || !ongoing()
					? activeSplineColor
					: inactiveSplineColor;
			};
			targetSplineDecoration.drawMode = GL_LINE_STRIP;
			targetSplineDecoration.renderLayer = RenderLayer::FarMidground;
			splineDecorationIdToSplineDef[targetSplineDecoration.getComponentId()] =
				splineDecorationIdToSplineDef[sourceSplineDecoration.getComponentId()];
			for (auto& controlPoint : splineDecorationIdToSplineDef[targetSplineDecoration.getComponentId()].controlPoints)
			{
				auto& controlPointDecoration = dynamicDecorations.emplace(Tools::Shapes2D::CreateVerticesOfCircle({ 0.0f, 0.0f }, controlPointRadius,
					controlPointComplexity));
				controlPoint.decorationId = controlPointDecoration.getComponentId();
				controlPointDecoration.colorF = [&, id = targetSplineDecoration.getComponentId()]() {
					return (activeSplineDecorationId && id == *activeSplineDecorationId)
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

		auto delete_ = [&]() {
			auto& dynamicDecorations = Globals::Components().decorations();

			for (const auto& controlPoint : splineDecorationIdToSplineDef[*activeSplineDecorationId].controlPoints)
				dynamicDecorations[controlPoint.decorationId].state = ComponentState::Outdated;

			dynamicDecorations[*activeSplineDecorationId].state = ComponentState::Outdated;
			splineDecorationIdToSplineDef.erase(*activeSplineDecorationId);
			activeSplineDecorationId = std::nullopt;
		};

		auto scaleActiveSpline = [&]() {
			auto& controlPoints = splineDecorationIdToSplineDef[*activeSplineDecorationId].controlPoints;
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

		if (mouse.pressing.rmb)
			removeControlPoint();
		else
			cameraMoving = false;

		if (mouse.pressing.mmb)
			tryAddOrMoveAdjacentControlPoint();
		else
			movingAdjacentControlPoint = std::nullopt;

		if (activeSplineDecorationId)
		{
			auto& splineDef = splineDecorationIdToSplineDef[*activeSplineDecorationId];

			if (keyboard.pressed[' '])
				splineDef.lightning = !splineDef.lightning;

			if (keyboard.pressed['L'])
				splineDef.loop = !splineDef.loop;

			if (keyboard.pressed['D'])
				deactivate();

			if (keyboard.pressed['C'])
				copy();

			if (keyboard.pressed[/*VK_DELETE*/0x2E])
				delete_();

			if (keyboard.pressing[/*VK_SHIFT*/0x10])
			{
				splineDef.complexity = std::clamp(splineDef.complexity + mouse.pressed.wheel, 1, 50);
			}

			if (keyboard.pressing[/*VK_CONTROL*/0x11] && mouse.pressed.wheel)
				scaleActiveSpline();
		}
	}

	void SplineEditing::update() const
	{
		for (auto& [splineDecorationId, SplineDef] : splineDecorationIdToSplineDef)
		{
			const auto& splineDef = splineDecorationIdToSplineDef.at(splineDecorationId);
			auto& splineDecoration = Globals::Components().decorations()[splineDecorationId];

			if (splineDef.controlPoints.size() < 2)
			{
				splineDecoration.vertices.clear();
				splineDecoration.state = ComponentState::Changed;
				continue;
			}

			std::vector<glm::vec2> controlPoints;
			controlPoints.reserve(splineDef.controlPoints.size());
			for (const auto& controlPoint : splineDef.controlPoints)
				controlPoints.push_back(controlPoint.pos);

			std::vector<glm::vec2> intermediateVeritces;

			const int numOfSplineVertices = splineDef.complexity * (controlPoints.size() - 1 + splineDef.loop) + 1;

			Tools::CubicHermiteSpline spline = splineDef.loop
				? Tools::CubicHermiteSpline(std::move(controlPoints), Tools::CubicHermiteSpline<>::loop)
				: Tools::CubicHermiteSpline(std::move(controlPoints));

			intermediateVeritces.reserve(numOfSplineVertices);

			for (int i = 0; i < numOfSplineVertices; ++i)
			{
				const float t = (float)i / (numOfSplineVertices - 1);
				intermediateVeritces.push_back(glm::vec3(spline.getInterpolation(t), 0.0f));
			}

			std::vector<glm::vec3> finalVertices;
			if (splineDef.lightning)
			{
				for (size_t i = 0; i < intermediateVeritces.size(); ++i)
				{
					if (!splineDef.loop && (i == 0 || i == intermediateVeritces.size() - 1))
						continue;

					auto& v = intermediateVeritces[i];

					auto vTransform = [&](const auto& prevV, const auto& v, const auto& nextV) {
						const float avgLength = (glm::distance(prevV, v) + glm::distance(v, nextV)) * 0.5f;
						const float rD = avgLength * splineDef.complexity * 0.005f;
						return std::remove_cvref<decltype(v)>::type(Tools::RandomFloat(-rD, rD), Tools::RandomFloat(-rD, rD));
					};

					if (splineDef.loop && i == 0)
					{
						const auto& prevV = intermediateVeritces[intermediateVeritces.size() - 2];
						const auto& nextV = intermediateVeritces[1];
						v += vTransform(prevV, v, nextV);
						continue;
					}

					if (splineDef.loop && i == intermediateVeritces.size() - 1)
					{
						v = intermediateVeritces.front();
						continue;
					}

					const auto& prevV = intermediateVeritces[i - 1];
					const auto& nextV = intermediateVeritces[i + 1];
					v += vTransform(prevV, v, nextV);
				}

				for (size_t i = 0; i < intermediateVeritces.size() - 1; ++i)
				{
					const float d = glm::distance(intermediateVeritces[i], intermediateVeritces[i + 1]);
					std::vector<glm::vec3> subVertices = Tools::Shapes2D::CreateVerticesOfLightning(intermediateVeritces[i], intermediateVeritces[i + 1], std::max(1, (int)d), 0.2f);
					finalVertices.insert(finalVertices.end(), subVertices.begin(), subVertices.end());
				}
			}
			else
				finalVertices = Tools::ConvertToVec3Vector(intermediateVeritces);

			splineDecoration.vertices = std::move(finalVertices);
			splineDecoration.state = ComponentState::Changed;
		}
	}

	void SplineEditing::generateCode(std::ofstream& fs) const
	{
		fs << "inline void CreateDeadlySplines(const Tools::PlayersHandler& playersHandler, std::unordered_set<ComponentId>& deadlySplines)\n";
		fs << "{\n";
		if (!splineDecorationIdToSplineDef.empty())
			fs << "	auto& polylines = Globals::Components().staticPolylines();\n";

		auto createControlPoints = [&](const auto& splineDef) {
			fs << "		std::vector<glm::vec2> controlPoints;\n";
			fs << "		controlPoints.reserve(" << splineDef.controlPoints.size() << ");\n";
			for (const auto& cp : splineDef.controlPoints)
				fs << "		controlPoints.push_back({" << cp.pos.x << ", " << cp.pos.y << "});\n";
		};

		for (const auto& [splineDecorationId, splineDef] : splineDecorationIdToSplineDef)
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
			fs << "			veritces.push_back(glm::vec3(spline.getInterpolation((float)i / (numOfVertices - 1)), 0.0f));\n";

			fs << "		polylines.emplace(std::move(veritces), Tools::BodyParams().sensor(true));\n";

			if (splineDef.lightning)
			{
				fs << "		polylines.last().segmentVerticesGenerator = [](const auto& v1, const auto& v2) { " <<
					"return Tools::Shapes2D::CreateVerticesOfLightning(v1, v2, std::max(1, (int)glm::distance(v1, v2)), 0.2f); };\n";
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
