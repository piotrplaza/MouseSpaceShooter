#include "splineTest.hpp"

#include <components/mouse.hpp>
#include <components/keyboard.hpp>
#include <components/decoration.hpp>
#include <components/screenInfo.hpp>
#include <components/camera.hpp>

#include <globals/components.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/glmHelpers.hpp>
#include <tools/splines.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <optional>
#include <list>

namespace Levels
{
	class SplineTest::Impl
	{
	public:
		void setup()
		{
			auto& staticDecoration = Globals::Components().staticDecorations();
			auto& dynamicDecoration = Globals::Components().dynamicDecorations();

			staticDecoration.emplace(Tools::CreateVerticesOfCircle({ 0.0f, 0.0f }, 0.05f, 20));
			staticDecoration.last().modelMatrixF = [this]() {
				return glm::translate(glm::mat4{ 1.0f }, { mousePos, 0.0f });
			};
			staticDecoration.last().renderLayer = RenderLayer::NearForeground;

			splineDecorationId = dynamicDecoration.emplace().getComponentId();
			dynamicDecoration.last().drawMode = GL_LINE_STRIP;
			dynamicDecoration.last().renderLayer = RenderLayer::Foreground;

			Globals::Components().camera().targetProjectionHSizeF = [&]() {
				return projectionHSize;
			};
		}

		void step()
		{
			const float controlPointSize = 0.1f;

			const auto& mouse = Globals::Components().mouse();
			const auto& keyboard = Globals::Components().keyboard();
			const auto& screenInfo = Globals::Components().screenInfo();
			const float screenRatio = (float)screenInfo.windowSize.x / screenInfo.windowSize.y;
			const glm::vec2 oldMousePos = mousePos;

			auto& dynamicDecorations = Globals::Components().dynamicDecorations();

			mousePos += mouse.getWorldSpaceDelta() * projectionHSize * 0.001f;
			mousePos.x = std::clamp(mousePos.x, -projectionHSize * screenRatio, projectionHSize * screenRatio);
			mousePos.y = std::clamp(mousePos.y, -projectionHSize, projectionHSize);

			const glm::vec2 mouseDelta = mousePos - oldMousePos;

			auto addControlPoint = [&](decltype(controlPoints)::iterator it) {
				dynamicDecorations.emplace(Tools::CreateVerticesOfCircle({ 0.0f, 0.0f }, controlPointSize, 20));
				dynamicDecorations.last().colorF = []() { return glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); };
				auto insertedIt = controlPoints.insert(it, { dynamicDecorations.last().getComponentId(), mousePos });
				dynamicDecorations.last().modelMatrixF = [&pos = insertedIt->second]() {
					return glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
				};
			};

			auto addOrMoveControlPoint = [&]() {
				if (!movingControlPoint)
					for (auto it = controlPoints.begin(); it != controlPoints.end(); ++it)
						if (glm::distance(it->second, oldMousePos) < controlPointSize)
						{
							movingControlPoint = it;
							break;
						}

				if (movingControlPoint)
				{
					(*movingControlPoint)->second += mouseDelta;
					return;
				}

				addControlPoint(controlPoints.end());
			};

			auto removeControlPoint = [&]() {
				for (auto it = controlPoints.begin(); it != controlPoints.end(); ++it)
					if (glm::distance(it->second, oldMousePos) < controlPointSize)
					{
						dynamicDecorations[it->first].state = ComponentState::Outdated;
						controlPoints.erase(it);
						break;
					}
			};

			auto tryAddPrevControlPoint = [&]() {
				for (auto it = controlPoints.begin(); it != controlPoints.end(); ++it)
					if (glm::distance(it->second, oldMousePos) < controlPointSize)
					{
						addControlPoint(it);
						break;
					}
			};

			auto updateSpline = [&]() {
				if (controlPoints.size() < 2)
					return;

				const int complexity = 10 * controlPoints.size();

				std::vector<glm::vec2> controlPoints;
				controlPoints.reserve(this->controlPoints.size());
				for (const auto& idAndPos : this->controlPoints)
					controlPoints.push_back(idAndPos.second);

				Tools::CubicHermiteSpline spline(std::move(controlPoints));
				std::vector<glm::vec3> splineVertices;
				splineVertices.reserve(complexity);
				for (int i = 0; i < complexity; ++i)
					splineVertices.push_back(glm::vec3(spline.getInterpolation((float)i / (complexity - 1)), 0.0f));
				auto& splineDecoration = Globals::Components().dynamicDecorations()[splineDecorationId];

				std::vector<glm::vec3> vertices;
				if (lightning)
					for (int i = 0; i < complexity - 1; ++i)
					{
						std::vector<glm::vec3> subVertices = Tools::CreateVerticesOfLightning(splineVertices[i], splineVertices[i + 1], 10);
						vertices.insert(vertices.end(), subVertices.begin(), subVertices.end());
					}
				else
					vertices = std::move(splineVertices);

				splineDecoration.vertices = std::move(vertices);
				splineDecoration.state = ComponentState::Changed;
			};

			if (mouse.pressing.lmb)
				addOrMoveControlPoint();
			else
				movingControlPoint = std::nullopt;

			if (mouse.pressed.rmb)
				removeControlPoint();

			if (mouse.pressed.mmb)
				tryAddPrevControlPoint();

			if (keyboard.pressed[' '])
				lightning = !lightning;

			updateSpline();

			projectionHSize = std::clamp(projectionHSize + mouse.pressed.wheel * -5.0f, 5.0f, 100.0f);
		}

	private:
		glm::vec2 mousePos{};
		std::list<std::pair<ComponentId, glm::vec2>> controlPoints;
		std::optional<decltype(controlPoints)::iterator> movingControlPoint;
		ComponentId splineDecorationId = 0;
		bool lightning = false;
		float projectionHSize = 10.0f;
	};

	SplineTest::SplineTest():
		impl(std::make_unique<Impl>())
	{
		impl->setup();
	}

	SplineTest::~SplineTest() = default;

	void SplineTest::step()
	{
		impl->step();
	}
}
