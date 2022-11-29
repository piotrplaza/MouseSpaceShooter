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
#include <tools/utility.hpp>

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
			auto& camera = Globals::Components().camera();

			staticDecoration.emplace(Tools::CreateVerticesOfCircle({ 0.0f, 0.0f }, 0.05f, 20));
			staticDecoration.last().modelMatrixF = [this]() {
				return glm::translate(glm::mat4{ 1.0f }, { mousePos, 0.0f });
			};
			staticDecoration.last().renderLayer = RenderLayer::NearForeground;

			splineDecorationId = dynamicDecoration.emplace().getComponentId();
			dynamicDecoration.last().drawMode = GL_LINE_STRIP;
			dynamicDecoration.last().renderLayer = RenderLayer::Foreground;

			camera.targetProjectionHSizeF = [&]() {
				return projectionHSize;
			};
			camera.targetPositionF = [&]() {
				return cameraPos;
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
			mousePos.x = std::clamp(mousePos.x, -projectionHSize * screenRatio + cameraPos.x, projectionHSize * screenRatio + cameraPos.x);
			mousePos.y = std::clamp(mousePos.y, -projectionHSize + cameraPos.y, projectionHSize + cameraPos.y);

			const glm::vec2 mouseDelta = mousePos - oldMousePos;

			auto addControlPoint = [&](decltype(controlPoints)::iterator it) {
				dynamicDecorations.emplace(Tools::CreateVerticesOfCircle({ 0.0f, 0.0f }, controlPointSize, 20));
				dynamicDecorations.last().colorF = []() { return glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); };
				auto insertedIt = controlPoints.insert(it, { dynamicDecorations.last().getComponentId(), mousePos });
				dynamicDecorations.last().modelMatrixF = [&pos = insertedIt->second]() {
					return glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
				};
				return insertedIt;
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

				addControlPoint(keyboard.pressing[/*VK_SHIFT*/0x10]
					? controlPoints.end()
					: controlPoints.begin());
			};

			auto removeControlPointOrMoveCamera = [&]() {
				if (movingCamera)
				{
					cameraPos += mouseDelta;
					return;
				}

				bool removed = false;
				for (auto it = controlPoints.begin(); it != controlPoints.end(); ++it)
					if (glm::distance(it->second, oldMousePos) < controlPointSize)
					{
						dynamicDecorations[it->first].state = ComponentState::Outdated;
						controlPoints.erase(it);
						removed = true;
						break;
					}

				if (!removed)
					movingCamera = true;
			};

			auto tryAddOrMovePrevControlPoint = [&]() {
				if (!movingPrevControlPoint)
					for (auto it = controlPoints.begin(); it != controlPoints.end(); ++it)
						if (glm::distance(it->second, oldMousePos) < controlPointSize)
						{
							movingPrevControlPoint = addControlPoint(keyboard.pressing[/*VK_SHIFT*/0x10]
								? std::next(it)
								: it);
							break;
						}

				if (movingPrevControlPoint)
					(*movingPrevControlPoint)->second += mouseDelta;
			};

			auto updateSpline = [&]() {
				auto& splineDecoration = Globals::Components().dynamicDecorations()[splineDecorationId];

				if (controlPoints.size() < 2)
				{
					splineDecoration.vertices.clear();
					splineDecoration.state = ComponentState::Changed;
					return;
				}

				const int complexity = 10 * controlPoints.size();

				std::vector<glm::vec2> controlPoints;
				controlPoints.reserve(this->controlPoints.size());
				for (const auto& idAndPos : this->controlPoints)
					controlPoints.push_back(idAndPos.second);

				Tools::CubicHermiteSpline spline = loop
					? Tools::CubicHermiteSpline(std::move(controlPoints), Tools::CubicHermiteSpline<>::loop)
					: Tools::CubicHermiteSpline(std::move(controlPoints));
				std::vector<glm::vec3> splineVertices;
				splineVertices.reserve(complexity);
				for (int i = 0; i < complexity; ++i)
				{
					const float t = (float)i / (complexity - 1);
					splineVertices.push_back(glm::vec3(lightning ? spline.getPostprocessedInterpolation(t, [rD = 0.1f](auto v) {
						return v + glm::vec2(Tools::Random(-rD, rD), Tools::Random(-rD, rD));
						}) : spline.getInterpolation(t), 0.0f));
				}

				std::vector<glm::vec3> vertices;
				if (lightning)
					for (int i = 0; i < complexity - 1; ++i)
					{
						std::vector<glm::vec3> subVertices = Tools::CreateVerticesOfLightning(splineVertices[i], splineVertices[i + 1], 10, 0.2f);
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

			if (mouse.pressing.rmb)
				removeControlPointOrMoveCamera();
			else
				movingCamera = false;

			if (mouse.pressing.mmb)
				tryAddOrMovePrevControlPoint();
			else
				movingPrevControlPoint = std::nullopt;

			if (keyboard.pressed[' '])
				lightning = !lightning;

			if (keyboard.pressed['L'])
				loop = !loop;

			updateSpline();

			projectionHSize = std::clamp(projectionHSize + mouse.pressed.wheel * -5.0f, 5.0f, 100.0f);
		}

	private:
		glm::vec2 mousePos{};
		glm::vec2 cameraPos{};
		std::list<std::pair<ComponentId, glm::vec2>> controlPoints;
		std::optional<decltype(controlPoints)::iterator> movingControlPoint;
		std::optional<decltype(controlPoints)::iterator> movingPrevControlPoint;
		bool movingCamera = false;
		ComponentId splineDecorationId = 0;
		bool lightning = false;
		bool loop = false;
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
