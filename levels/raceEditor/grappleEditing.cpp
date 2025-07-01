#include "grappleEditing.hpp"

#include <components/keyboard.hpp>
#include <components/mouse.hpp>
#include <components/decoration.hpp>
#include <components/grapple.hpp>

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
	constexpr glm::vec4 grappleRangeColor(0.5f);
	constexpr float radiusScalingFactor = 0.5f;
	constexpr float rangeScalingFactor = 2.0f;
	constexpr float minGrappleRadius = 0.2f;
	constexpr float maxGrappleRadius = 100.0f;
	constexpr float minGrappleRange = 5.0f;
	constexpr float maxGrappleRange = 1000.0f;
}

namespace Levels
{
	GrappleEditing::GrappleEditing(const glm::vec2& mousePos, const glm::vec2& oldMousePos, const glm::vec2& mouseDelta, const float& zoomScale, FBool ongoing, const Tools::ParamsFromFile& paramsFromFile) :
		mousePos(mousePos),
		oldMousePos(oldMousePos),
		mouseDelta(mouseDelta),
		zoomScale(zoomScale),
		ongoing(std::move(ongoing)),
		paramsFromFile(paramsFromFile)
	{
		paramsFromFile.loadParam(controlPointRadius, "editor.controlPointRadius", false);
		paramsFromFile.loadParam(grappleColor, "grapples.color", false);
		paramsFromFile.loadParam(grappleInitRadius, "grapples.initRadius", false);
		paramsFromFile.loadParam(grappleInitRange, "grapples.initRange", false);
	}

	void GrappleEditing::edit()
	{
		const auto& mouse = Globals::Components().mouse();
		const auto& keyboard = Globals::Components().keyboard();
		auto& dynamicDecorations = Globals::Components().decorations();
		auto& grapples = Globals::Components().grapples();

		auto addControlPoint = [&]() {
			auto& cpDecorations = dynamicDecorations.emplace(Tools::Shapes2D::CreatePositionsOfDisc({ 0.0f, 0.0f }, controlPointRadius, 20));
			cpDecorations.colorF = controlPointColor;
			const auto& controlPoint = controlPoints.emplace_back(cpDecorations.getComponentId(), mousePos);
			const auto cpIt = std::prev(controlPoints.end());
			cpDecorations.modelMatrixF = [&, &pos = controlPoint.pos]() {
				return glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)), glm::vec3(zoomScale));
			};
			cpDecorations.renderF = ongoing;
			cpDecorations.renderLayer = RenderLayer::NearMidground;

			auto& grapple = grapples.emplace(Tools::CreateDiscBody(grappleInitRadius, Tools::BodyParams{}));
			grapple.range = grappleInitRange;
			grapple.stepF = [&]() {
				grapple.setOrigin(controlPoint.pos);
				grapple.colorF = grappleColor;
			};

			auto& rangeDecoration = dynamicDecorations.emplace(Tools::Shapes2D::CreatePositionsOfCircle({ 0.0f, 0.0f }, 1.0f, 100));
			rangeDecoration.drawMode = GL_LINE_LOOP;
			rangeDecoration.colorF = [&]() {
				return grappleRangeColor * (float)ongoing();
			};

			const auto& grappleData = cpDecoIdsToGrapplesData.insert({ controlPoint.decorationId, GrappleData{cpIt, grapple.getComponentId(), rangeDecoration.getComponentId(), grappleInitRadius, grappleInitRange} }).first->second;

			rangeDecoration.modelMatrixF = [&, &pos = controlPoint.pos]() {
				return glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)), glm::vec3(grappleData.range));
			};
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
			controlPointAction([&](auto it) {
				if (movingControlPoint)
					return;

				auto cpDecoIdsToGrapplesDataIt = cpDecoIdsToGrapplesData.find(it->decorationId);
				grapples[cpDecoIdsToGrapplesDataIt->second.grappleId].state = ComponentState::Outdated;
				dynamicDecorations[cpDecoIdsToGrapplesDataIt->second.rangeDecorationId].state = ComponentState::Outdated;
				dynamicDecorations[it->decorationId].state = ComponentState::Outdated;
				controlPoints.erase(it);
				cpDecoIdsToGrapplesData.erase(cpDecoIdsToGrapplesDataIt);
			});
		};

		auto resizeGrapple = [&]() {
			controlPointAction([&](auto it) {
				auto& grappleData = cpDecoIdsToGrapplesData.at(it->decorationId);
				float newRadius = std::clamp(grappleData.radius + mouse.pressed.wheel * radiusScalingFactor * zoomScale, minGrappleRadius, maxGrappleRadius);
				if (newRadius != grappleData.radius)
				{
					grappleData.radius = newRadius;
					auto& grapple = Globals::Components().grapples()[grappleData.grappleId];
					grapple.changeBody(Tools::CreateDiscBody(newRadius, Tools::BodyParams{}));
				}
			});
		};

		auto resizeRange = [&]() {
			controlPointAction([&](auto it) {
				auto& grappleData = cpDecoIdsToGrapplesData.at(it->decorationId);
				grappleData.range = std::clamp(grappleData.range + mouse.pressed.wheel * rangeScalingFactor * zoomScale, minGrappleRange, maxGrappleRange);
				auto& grapple = Globals::Components().grapples()[grappleData.grappleId];
				grapple.range = grappleData.range;
			});
		};

		if (mouse.pressing.lmb)
			addOrMoveControlPoint();
		else
			movingControlPoint = std::nullopt;

		if (mouse.pressing.rmb && keyboard.pressing[/*VK_SHIFT*/0x10])
			removeControlPoint();

		if (mouse.pressed.wheel && keyboard.pressing[/*VK_SHIFT*/0x10])
			resizeGrapple();

		if (mouse.pressed.wheel && keyboard.pressing[/*VK_CONTROL*/0x11])
			resizeRange();
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
				"Tools::BodyParams{}.position({ (float)" << grappleData.controlPoint->pos.x << ", (float)" << grappleData.controlPoint->pos.y << " }))).range = (float)" << grappleData.range << ";\n";
			fs << "	Globals::Components().staticGrapples().last().colorF = glm::vec4((float)" << grappleColor.r << ", (float)" << grappleColor.g << ", (float)" << grappleColor.b << ", (float)" << grappleColor.a << ");\n";

		}
		fs << "}\n";
		fs << "\n";
	}
}
