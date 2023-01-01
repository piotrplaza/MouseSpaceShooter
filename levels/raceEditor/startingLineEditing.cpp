#include "startingLineEditing.hpp"

#include <components/keyboard.hpp>
#include <components/mouse.hpp>
#include <components/decoration.hpp>
#include <components/wall.hpp>

#include <globals/components.hpp>

#include <tools/b2Helpers.hpp>

#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <numeric>
#include <algorithm>

namespace
{
	constexpr glm::vec4 controlPointColor(0.0f, 1.0f, 0.0f, 1.0f);
	constexpr glm::vec4 startingLineColor(1.0f, 0.0f, 0.0f, 1.0f);
	constexpr glm::vec4 startingLineEndsColor(1.0f, 0.0f, 0.0f, 1.0f);
	constexpr glm::vec4 arrowColor(0.4f, 0.4f, 0.4f, 1.0f);
	constexpr float controlPointRadius = 0.75f;
	constexpr float scalingFactor = 0.05f;
}

namespace Levels
{
	StartingLineEditing::StartingLineEditing(const glm::vec2& mousePos, const glm::vec2& oldMousePos, const glm::vec2& mouseDelta, const float& zoomScale, std::function<bool()> ongoing):
		mousePos(mousePos),
		oldMousePos(oldMousePos),
		mouseDelta(mouseDelta),
		zoomScale(zoomScale),
		ongoing(std::move(ongoing))
	{
		auto& dynamicDecorations = Globals::Components().dynamicDecorations();

		auto& startingLine = dynamicDecorations.emplace();
		startingLine.renderF = [&]() { return controlPoints.size() == 2; };
		startingLine.colorF = []() { return startingLineColor; };
		startingLine.drawMode = GL_LINES;
		startingLine.renderLayer = RenderLayer::FarMidground;
		startingLineId = startingLine.getComponentId();

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
			return glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)), angle, { 0.0f, 0.0f, 1.0f }), glm::vec3(zoomScale));
		};
		arrow.drawMode = GL_LINES;
		arrow.renderLayer = RenderLayer::FarMidground;

		auto& startingLineDecoration = dynamicDecorations.emplace();

		auto& dynamicWalls = Globals::Components().dynamicWalls();

		for (int i = 0; i < 2; ++i)
		{
			startingLineEnds[i] = dynamicWalls.emplace(Tools::CreateCircleBody(startingLineEndsRadius)).getComponentId();
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

	void StartingLineEditing::edit(bool& cameraMoving)
	{
		const auto& mouse = Globals::Components().mouse();
		const auto& keyboard = Globals::Components().keyboard();
		auto& dynamicDecorations = Globals::Components().dynamicDecorations();

		auto addControlPoint = [&]() {
			dynamicDecorations.emplace(Tools::CreateVerticesOfCircle({ 0.0f, 0.0f }, controlPointRadius, 20));
			dynamicDecorations.last().colorF = []() { return controlPointColor; };
			const auto& controlPoint = controlPoints.emplace_back( dynamicDecorations.last().getComponentId(), mousePos );
			dynamicDecorations.last().modelMatrixF = [&, &pos = controlPoint.pos]() {
				return glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)), glm::vec3(zoomScale));
			};
			dynamicDecorations.last().renderF = ongoing;
			dynamicDecorations.last().renderLayer = RenderLayer::NearMidground;

			if (controlPoints.size() > 2)
			{
				dynamicDecorations[controlPoints.front().decorationId].state = ComponentState::Outdated;
				controlPoints.pop_front();
			}
		};

		auto controlPointAction = [&](std::function<void(std::list<ControlPoint>::iterator)> action)
		{
			for (auto it = controlPoints.begin(); it != controlPoints.end(); ++it)
				if (glm::distance(it->pos, oldMousePos) < controlPointRadius * zoomScale)
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
						controlPoint.pos += mouseDelta;
				}
				else
					(*movingControlPoint)->pos += mouseDelta;

				return;
			}

			addControlPoint();
		};

		auto removeControlPoint = [&]() {
			if (cameraMoving)
				return;

			bool removed = false;
			controlPointAction([&](auto it) {
				if (movingControlPoint)
					return;

				if (keyboard.pressing[/*VK_SHIFT*/0x10])
				{
					for (const auto& controlPoint : controlPoints)
						dynamicDecorations[controlPoint.decorationId].state = ComponentState::Outdated;
					controlPoints.clear();
				}
				else
				{
					dynamicDecorations[it->decorationId].state = ComponentState::Outdated;
					controlPoints.erase(it);
				}

				removed = true;
			});

			if (!removed)
				cameraMoving = true;
		};

		auto changeStartingLineEndsRadius = [&]() {
			if (!mouse.pressed.wheel)
				return;

			startingLineEndsRadius = std::clamp(startingLineEndsRadius + mouse.pressed.wheel * 0.2f, 0.2f, 5.0f);
			auto& dynamicWalls = Globals::Components().dynamicWalls();
			for (auto wallId : startingLineEnds)
				dynamicWalls[wallId].changeBody(Tools::CreateCircleBody(startingLineEndsRadius));
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

		if (keyboard.pressed['E'])
			controlPointsEnds = !controlPointsEnds;

		if (mouse.pressing.lmb)
			addOrMoveControlPoint();
		else
			movingControlPoint = std::nullopt;

		if (mouse.pressing.rmb)
			removeControlPoint();
		else
			cameraMoving = false;

		if (keyboard.pressing[/*VK_SHIFT*/0x10])
			changeStartingLineEndsRadius();

		if (keyboard.pressing[/*VK_CONTROL*/0x11] && mouse.pressed.wheel)
			scaleStartingLine();
	}

	void StartingLineEditing::update() const
	{
		auto& startingLine = Globals::Components().dynamicDecorations()[startingLineId];

		startingLine.vertices.clear();
		for (auto& controlPoint : controlPoints)
			startingLine.vertices.emplace_back(controlPoint.pos, 0.0f);
		startingLine.state = ComponentState::Changed;
	}

	void StartingLineEditing::generateCode(std::ofstream& fs) const
	{
	}
}
