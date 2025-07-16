#include "startingLineEditing.hpp"

#include <components/keyboard.hpp>
#include <components/mouse.hpp>
#include <components/decoration.hpp>
#include <components/wall.hpp>

#include <globals/components.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/Shapes2D.hpp>
#include <tools/paramsFromFile.hpp>

#include <glm/vec4.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <numeric>
#include <algorithm>

namespace
{
	constexpr glm::vec4 controlPointColor(0.0f, 1.0f, 0.0f, 1.0f);
	constexpr glm::vec4 startingPositionLineColor(0.4f, 0.4f, 0.4f, 1.0f);
	constexpr glm::vec4 startingLineEndsColor(1.0f, 0.0f, 0.0f, 1.0f);
	constexpr glm::vec4 arrowColor(0.4f, 0.4f, 0.4f, 1.0f);
	constexpr float scalingFactor = 0.05f;
}

namespace Levels
{
	StartingLineEditing::StartingLineEditing(const glm::vec2& mousePos, const glm::vec2& oldMousePos, const glm::vec2& mouseDelta, const float& zoomScale, const float& scale, FBool ongoing, const Tools::ParamsFromFile& paramsFromFile):
		mousePos(mousePos),
		oldMousePos(oldMousePos),
		mouseDelta(mouseDelta),
		zoomScale(zoomScale),
		scale(scale),
		ongoing(std::move(ongoing)),
		paramsFromFile(paramsFromFile)
	{
		paramsFromFile.loadParam(controlPointRadius, "editor.controlPointRadius", false);
		paramsFromFile.loadParam(startingLineColor, "startingLine.color", false);

		auto& dynamicDecorations = Globals::Components().decorations();

		auto& startingLine = dynamicDecorations.emplace();
		startingLine.renderF = [&]() { return controlPoints.size() == 2; };
		startingLine.colorF = startingLineColor;
		startingLine.modelMatrixF = [&]() {
			return glm::scale(glm::mat4(1.0f), glm::vec3(scale));
		};
		startingLine.drawMode = GL_LINES;
		startingLine.renderLayer = RenderLayer::FarMidground;
		startingLineId = startingLine.getComponentId();

		auto& startingPositionLine = dynamicDecorations.emplace();
		startingPositionLine.renderF = [&]() { return controlPoints.size() == 2 && this->ongoing(); };
		startingPositionLine.colorF = startingPositionLineColor;
		startingPositionLine.modelMatrixF = [&]() {
			const glm::vec2 v = glm::rotate(glm::normalize(controlPoints.back().pos - controlPoints.front().pos), -glm::half_pi<float>());
			return glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(v * startingPositionLineDistance, 0.0f)), glm::vec3(scale));
		};
		startingPositionLine.drawMode = GL_LINES;
		startingPositionLine.renderLayer = RenderLayer::FarMidground;
		startingPositionLineId = startingPositionLine.getComponentId();

		auto& arrow = dynamicDecorations.emplace(std::vector<glm::vec3>{
			{0.0f, 0.0f, 0.0f}, { 2.0f, 0.0f, 0.0f },
			{ 2.0f, 0.0f, 0.0f }, { 1.0f, 0.5f, 0.0f },
			{ 2.0f, 0.0f, 0.0f }, { 1.0f, -0.5f, 0.0f }
		});
		arrow.renderF = [&]() { return controlPoints.size() == 2 && this->ongoing(); };
		arrow.colorF = []() { return arrowColor; };
		arrow.modelMatrixF = [&]() {
			const glm::vec2 pos = (controlPoints.front().pos + controlPoints.back().pos) * 0.5f;
			const glm::vec2 v = glm::normalize(controlPoints.back().pos - controlPoints.front().pos);
			const float angle = glm::orientedAngle({ 1.0f, 0.0f }, v) + glm::half_pi<float>();
			return glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(pos * scale, 0.0f)), angle, { 0.0f, 0.0f, 1.0f }), glm::vec3(zoomScale));
		};
		arrow.drawMode = GL_LINES;
		arrow.renderLayer = RenderLayer::FarMidground;
		arrowId = arrow.getComponentId();

		auto& startingLineDecoration = dynamicDecorations.emplace();

		auto& dynamicWalls = Globals::Components().walls();

		for (int i = 0; i < 2; ++i)
		{
			startingLineEnds[i] = dynamicWalls.emplace(Tools::CreateDiscBody(startingLineEndsRadius)).getComponentId();
			dynamicWalls.last().renderF = [&]() { return controlPointsEnds && controlPoints.size() == 2; };
			dynamicWalls.last().colorF = []() { return startingLineEndsColor; };
			dynamicWalls.last().stepF = [&, &wall = dynamicWalls.last(), i]() {
				if (controlPoints.size() != 2)
				{
					wall.body->SetEnabled(false);
					return;
				}

				wall.body->SetEnabled(controlPointsEnds);
				wall.body->SetTransform(ToVec2<b2Vec2>(i == 0 ? controlPoints.front().pos : controlPoints.back().pos), wall.body->GetAngle());
			};
		}
	}

	void StartingLineEditing::edit()
	{
		const auto& mouse = Globals::Components().mouse();
		const auto& keyboard = Globals::Components().keyboard();
		auto& dynamicDecorations = Globals::Components().decorations();

		auto addControlPoint = [&]() {
			auto& cpDecoration = dynamicDecorations.emplace(Tools::Shapes2D::CreatePositionsOfDisc({ 0.0f, 0.0f }, controlPointRadius, 20));
			cpDecoration.colorF = []() { return controlPointColor; };
			const auto& controlPoint = controlPoints.emplace_back(cpDecoration.getComponentId(), mousePos / scale);
			cpDecoration.modelMatrixF = [&, &pos = controlPoint.pos]() {
				return glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(pos * scale, 0.0f)), glm::vec3(zoomScale));
			};
			cpDecoration.renderF = ongoing;
			cpDecoration.renderLayer = RenderLayer::NearMidground;

			if (controlPoints.size() > 2)
			{
				for (const auto& controlPoint : controlPoints)
					cpDecoration.state = ComponentState::Outdated;
				controlPoints.clear();
			}
		};

		auto controlPointAction = [&](std::function<void(std::list<ControlPoint>::iterator)> action)
		{
			for (auto it = controlPoints.begin(); it != controlPoints.end(); ++it)
				if (glm::distance(it->pos * scale, oldMousePos) < controlPointRadius * zoomScale)
				{
					action(it);
					return;
				}
		};

		auto addOrMoveControlPoint = [&]() {
			if (!movingControlPoint)
				controlPointAction([&](auto it) {
					movingControlPoint = it;
				});

			if (movingControlPoint)
			{
				if (keyboard.pressing[/*VK_SHIFT*/0x10])
				{
					for (auto& controlPoint : controlPoints)
						controlPoint.pos += mouseDelta / scale;
				}
				else
					(*movingControlPoint)->pos += mouseDelta / scale;

				return;
			}

			addControlPoint();
		};

		auto removeControlPoint = [&]() {
			controlPointAction([&](auto it) {
				if (movingControlPoint)
					return;

				dynamicDecorations[it->decorationId].state = ComponentState::Outdated;
				controlPoints.erase(it);
			});
		};

		auto removeControlPoints = [&]() {
			controlPointAction([&](auto it) {
				if (movingControlPoint)
					return;

				for (const auto& controlPoint : controlPoints)
					dynamicDecorations[controlPoint.decorationId].state = ComponentState::Outdated;
				controlPoints.clear();
			});
		};

		auto changeStartingPositionLineDistance = [&]() {
			startingPositionLineDistance += mouse.pressed.wheel * 2.0f;
			startingPositionLineDistance = std::clamp(startingPositionLineDistance, 0.0f, 100.0f);
		};

		auto scaleStartingLine = [&]() {
			const auto center = std::accumulate(controlPoints.begin(), controlPoints.end(), glm::vec2(0.0f), [](auto sum, const auto& cp) {
				return sum + cp.pos; }) / (float)controlPoints.size();

			for (auto& cp : controlPoints)
			{
				auto d = cp.pos - center;
				cp.pos = center + d * (1.0f + mouse.pressed.wheel * scalingFactor);
			}
		};

		auto changeStartingLineEndsRadius = [&]() {
			startingLineEndsRadius = std::clamp(startingLineEndsRadius + mouse.pressed.wheel * 0.2f, 0.0f, 20.0f);
			auto& dynamicWalls = Globals::Components().walls();
			for (auto wallId : startingLineEnds)
				dynamicWalls[wallId].changeBody(Tools::CreateDiscBody(startingLineEndsRadius));
		};

		auto reverse = [&]() {
			std::reverse(controlPoints.begin(), controlPoints.end());
			std::reverse(startingLineEnds.begin(), startingLineEnds.end());
		};

		if (keyboard.pressed['E'])
			controlPointsEnds = !controlPointsEnds;

		if (mouse.pressed.mmb)
			reverse();

		if (mouse.pressing.lmb)
			addOrMoveControlPoint();
		else
			movingControlPoint = std::nullopt;

		if (mouse.pressing.rmb && keyboard.pressing[/*VK_SHIFT*/0x10])
			removeControlPoint();

		if (mouse.pressing.rmb && keyboard.pressing[/*VK_CONTROL*/0x11])
			removeControlPoints();

		if (mouse.pressed.wheel && keyboard.pressing[/*VK_SHIFT*/0x10])
			changeStartingPositionLineDistance();

		if (mouse.pressed.wheel && keyboard.pressing[/*VK_CONTROL*/0x11])
			scaleStartingLine();

		if (mouse.pressed.wheel && keyboard.pressing[/*VK_MENU*/0x12])
			changeStartingLineEndsRadius();
	}

	void StartingLineEditing::update() const
	{
		auto& startingLine = Globals::Components().decorations()[startingLineId];
		auto& startingPositionLine = Globals::Components().decorations()[startingPositionLineId];
		auto& arrow = Globals::Components().decorations()[arrowId];

		const bool lineReady = controlPoints.size() == 2;

		startingLine.setEnabled(lineReady);
		startingPositionLine.setEnabled(lineReady);
		arrow.setEnabled(lineReady);

		if (!lineReady)
			return;

		startingLine.positions.clear();
		for (auto& controlPoint : controlPoints)
			startingLine.positions.emplace_back(controlPoint.pos, 0.0f);
		startingLine.state = ComponentState::Changed;

		startingPositionLine.positions = startingLine.positions;
		startingPositionLine.state = ComponentState::Changed;
	}

	void StartingLineEditing::generateCode(std::ofstream& fs) const
	{
		fs << "inline void CreateStartingLine(ComponentId& startingLineId, glm::vec2& p1, glm::vec2& p2, float& startingPositionLineDistance)\n";
		fs << "{\n";
		if (controlPoints.size() == 2)
		{
			fs << "	const glm::vec2 cp1 = { " << controlPoints.front().pos.x << ", " << controlPoints.front().pos.y << " };\n";
			fs << "	const glm::vec2 cp2 = { " << controlPoints.back().pos.x << ", " << controlPoints.back().pos.y << " };\n";
			fs << "	auto& startingLine = Globals::Components().staticPolylines().emplace(std::vector<glm::vec2>{ cp1 * scale, cp2 * scale },\n";
			fs << "		Tools::BodyParams().sensor(true)); \n";
			fs << "	startingLine.colorF = glm::vec4((float)" << startingLineColor.r << ", (float)" << startingLineColor.g << ", (float)" << startingLineColor.b << ", (float)" << startingLineColor.a << ");\n";
			fs << "\n";
			fs << "	startingLineId = startingLine.getComponentId();\n";
			fs << "	p1 = cp1 * scale;\n";
			fs << "	p2 = cp2 * scale;\n";
			fs << "	startingPositionLineDistance = (float)" << startingPositionLineDistance << ";\n";
		}
		fs << "}\n";
		fs << "\n";
	}

	std::vector<glm::vec2> StartingLineEditing::getStartingLineEnds() const
	{
		if (controlPoints.size() < 2)
			return {};

		return { controlPoints.front().pos, controlPoints.back().pos };
	}

	float StartingLineEditing::getStartingPositionLineDistance() const
	{
		return startingPositionLineDistance;
	}
}
