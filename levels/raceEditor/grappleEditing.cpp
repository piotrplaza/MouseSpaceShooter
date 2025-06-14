#include "grappleEditing.hpp"

#include <components/keyboard.hpp>
#include <components/mouse.hpp>
#include <components/decoration.hpp>
#include <components/grapple.hpp>

#include <globals/components.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/Shapes2D.hpp>

#include <glm/vec4.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <numeric>
#include <algorithm>

namespace
{
	constexpr glm::vec4 controlPointColor(0.0f, 1.0f, 0.0f, 1.0f);
	constexpr glm::vec4 grappleColor(0.0f, 1.0f, 1.0f, 1.0f);
	constexpr float controlPointRadius = 0.75f;
	constexpr float grappleInitRadius = 2.0f;
	constexpr float grappleInitRange = 40.0f;
	constexpr float scalingFactor = 0.5f;
	constexpr float minGrappleRadius = 0.2f;
	constexpr float maxGrappleRadius = 100.0f;
}

namespace Levels
{
	GrappleEditing::GrappleEditing(const glm::vec2& mousePos, const glm::vec2& oldMousePos, const glm::vec2& mouseDelta, const float& zoomScale, FBool ongoing) :
		mousePos(mousePos),
		oldMousePos(oldMousePos),
		mouseDelta(mouseDelta),
		zoomScale(zoomScale),
		ongoing(std::move(ongoing))
	{
	}

	void GrappleEditing::edit(bool& cameraMoving)
	{
		const auto& mouse = Globals::Components().mouse();
		const auto& keyboard = Globals::Components().keyboard();
		auto& dynamicDecorations = Globals::Components().decorations();
		auto& grapples = Globals::Components().grapples();

		auto addControlPoint = [&]() {
			dynamicDecorations.emplace(Tools::Shapes2D::CreatePositionsOfCircle({ 0.0f, 0.0f }, controlPointRadius, 20));
			dynamicDecorations.last().colorF = controlPointColor;
			const auto& controlPoint = controlPoints.emplace_back(dynamicDecorations.last().getComponentId(), mousePos);
			const auto cpIt = std::prev(controlPoints.end());
			dynamicDecorations.last().modelMatrixF = [&, &pos = controlPoint.pos]() {
				return glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)), glm::vec3(zoomScale));
			};
			dynamicDecorations.last().renderF = ongoing;
			dynamicDecorations.last().renderLayer = RenderLayer::NearMidground;

			auto& grapple = grapples.emplace(Tools::CreateDiscBody(grappleInitRadius, Tools::BodyParams{}));
			grapple.range = grappleInitRange;
			grapple.stepF = [&]() {
				grapple.setOrigin(controlPoint.pos);
				grapple.colorF = grappleColor;
			};

			cpDecoIdsToGrapplesData.insert({ controlPoint.decorationId, GrappleData{cpIt, grapple.getComponentId(), grappleInitRadius, grappleInitRange} });
		};

		auto controlPointAction = [&](std::function<void(std::list<ControlPoint>::iterator)> action) {
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
					{
						const auto& grappleData = cpDecoIdsToGrapplesData.at(controlPoint.decorationId);
						grapples[grappleData.grappleId].state = ComponentState::Outdated;
						dynamicDecorations[controlPoint.decorationId].state = ComponentState::Outdated;
					}
					controlPoints.clear();
					cpDecoIdsToGrapplesData.clear();
				}
				else
				{
					auto cpDecoIdsToGrapplesDataIt = cpDecoIdsToGrapplesData.find(it->decorationId);
					grapples[cpDecoIdsToGrapplesDataIt->second.grappleId].state = ComponentState::Outdated;
					dynamicDecorations[it->decorationId].state = ComponentState::Outdated;
					controlPoints.erase(it);
					cpDecoIdsToGrapplesData.erase(cpDecoIdsToGrapplesDataIt);
				}
				removed = true;
			});

			if (!removed)
				cameraMoving = true;
		};

		auto resizeGrapple = [&]() {
			if (mouse.pressed.wheel == 0)
				return;
			controlPointAction([&](auto it) {
				auto& grappleData = cpDecoIdsToGrapplesData.at(it->decorationId);
				float newRadius = std::clamp(grappleData.radius + mouse.pressed.wheel * scalingFactor * zoomScale, minGrappleRadius, maxGrappleRadius);
				if (newRadius != grappleData.radius)
				{
					grappleData.radius = newRadius;
					auto& grapple = Globals::Components().grapples()[grappleData.grappleId];
					grapple.changeBody(Tools::CreateDiscBody(newRadius, Tools::BodyParams{}));
				}
			});
		};

		if (mouse.pressing.lmb)
			addOrMoveControlPoint();
		else
			movingControlPoint = std::nullopt;

		if (mouse.pressing.rmb)
			removeControlPoint();
		else
			cameraMoving = false;

		if (keyboard.pressing[/*VK_SHIFT*/0x10])
			resizeGrapple();
	}

	void GrappleEditing::update() const
	{
	}

	void GrappleEditing::generateCode(std::ofstream& fs) const
	{
		fs << "inline void CreateGrapples()\n";
		fs << "{\n";
		for (const auto&[decoId, grappleData]: cpDecoIdsToGrapplesData)
		{
			fs << "	Globals::Components().staticGrapples().emplace(Tools::CreateDiscBody((float)" << grappleData.radius << ", " <<
				"Tools::BodyParams{}.position({ (float)" << grappleData.controlPoint->pos.x << ", (float)" << grappleData.controlPoint->pos.y << " }))).range = (float)" << grappleData.range << "; \n";

		}
		fs << "}\n";
		fs << "\n";
	}
}
