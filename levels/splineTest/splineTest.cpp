#include "splineTest.hpp"

#include <components/mouse.hpp>
#include <components/keyboard.hpp>
#include <components/decoration.hpp>
#include <components/screenInfo.hpp>
#include <components/camera2D.hpp>

#include <globals/components.hpp>

#include <tools/Shapes2D.hpp>
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
			auto& dynamicDecorations = Globals::Components().dynamicDecorations();
			auto& camera = Globals::Components().camera2D();

			staticDecoration.emplace(Tools::Shapes2D::CreateVerticesOfCircle({ 0.0f, 0.0f }, 0.05f, 20));
			staticDecoration.last().modelMatrixF = [this]() {
				return glm::translate(glm::mat4{ 1.0f }, { mousePos, 0.0f });
			};
			staticDecoration.last().renderLayer = RenderLayer::NearForeground;

			splineDecorationId = dynamicDecorations.emplace().getComponentId();
			dynamicDecorations.last().drawMode = GL_LINE_STRIP;
			dynamicDecorations.last().renderLayer = RenderLayer::Foreground;

			camera.targetPositionAndProjectionHSizeF = [&]() {
				return glm::vec3(cameraPos, projectionHSize);
			};
		}

		void step()
		{
			const float controlPointSize = 0.1f;

			const auto& mouse = Globals::Components().mouse();
			const auto& keyboard = Globals::Components().keyboard();
			const auto& screenInfo = Globals::Components().screenInfo();
			const float screenRatio = screenInfo.getAspectRatio();
			const glm::vec2 oldMousePos = mousePos;

			auto& dynamicDecorations = Globals::Components().dynamicDecorations();

			mousePos += mouse.getCartesianDelta() * projectionHSize * 0.001f;
			mousePos.x = std::clamp(mousePos.x, -projectionHSize * screenRatio + cameraPos.x, projectionHSize * screenRatio + cameraPos.x);
			mousePos.y = std::clamp(mousePos.y, -projectionHSize + cameraPos.y, projectionHSize + cameraPos.y);

			const glm::vec2 mouseDelta = mousePos - oldMousePos;

			auto addControlPoint = [&](decltype(controlPoints)::iterator it) {
				dynamicDecorations.emplace(Tools::Shapes2D::CreateVerticesOfCircle({ 0.0f, 0.0f }, controlPointSize, 20));
				dynamicDecorations.last().colorF = []() { return glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); };
				auto insertedIt = controlPoints.insert(it, { dynamicDecorations.last().getComponentId(), mousePos });
				dynamicDecorations.last().modelMatrixF = [&pos = insertedIt->pos]() {
					return glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
				};
				return insertedIt;
			};

			auto addOrMoveControlPoint = [&]() {
				if (!movingControlPoint)
					for (auto it = controlPoints.begin(); it != controlPoints.end(); ++it)
						if (glm::distance(it->pos, oldMousePos) < controlPointSize)
						{
							movingControlPoint = it;
							break;
						}

				if (movingControlPoint)
				{
					(*movingControlPoint)->pos += mouseDelta;
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
					if (glm::distance(it->pos, oldMousePos) < controlPointSize)
					{
						dynamicDecorations[it->decorationId].state = ComponentState::Outdated;
						controlPoints.erase(it);
						removed = true;
						break;
					}

				if (!removed)
					movingCamera = true;
			};

			auto tryAddOrMoveAdjacentControlPoint = [&]() {
				if (!movingAdjacentControlPoint)
					for (auto it = controlPoints.begin(); it != controlPoints.end(); ++it)
						if (glm::distance(it->pos, oldMousePos) < controlPointSize)
						{
							movingAdjacentControlPoint = addControlPoint(keyboard.pressing[/*VK_SHIFT*/0x10]
								? std::next(it)
								: it);
							break;
						}

				if (movingAdjacentControlPoint)
					(*movingAdjacentControlPoint)->pos += mouseDelta;
			};

			auto updateSpline = [&]() {
				auto& splineDecoration = Globals::Components().dynamicDecorations()[splineDecorationId];

				if (controlPoints.size() < 2)
				{
					splineDecoration.vertices.clear();
					splineDecoration.state = ComponentState::Changed;
					return;
				}

				std::vector<glm::vec2> controlPoints;
				controlPoints.reserve(this->controlPoints.size());
				for (const auto& controlPoint : this->controlPoints)
					controlPoints.push_back(controlPoint.pos);

				std::vector<glm::vec2> intermediateVeritces;

				const int numOfSplineVertices = complexity * (controlPoints.size() - 1 + loop) + 1;

				Tools::CubicHermiteSpline spline = loop
					? Tools::CubicHermiteSpline(std::move(controlPoints), Tools::CubicHermiteSpline<>::loop)
					: Tools::CubicHermiteSpline(std::move(controlPoints));

				intermediateVeritces.reserve(numOfSplineVertices);

				for (int i = 0; i < numOfSplineVertices; ++i)
				{
					const float t = (float)i / (numOfSplineVertices - 1);
					intermediateVeritces.push_back(glm::vec3(spline.getInterpolation(t), 0.0f));
				}

				std::vector<glm::vec3> finalVertices;
				if (lightning)
				{
					for (auto& v : intermediateVeritces)
					{
						const float rD = 0.1f;
						v += glm::vec2(Tools::RandomFloat(-rD, rD), Tools::RandomFloat(-rD, rD));
					}

					for (size_t i = 0; i < intermediateVeritces.size() - 1; ++i)
					{
						std::vector<glm::vec3> subVertices = Tools::Shapes2D::CreateVerticesOfLightning(intermediateVeritces[i], intermediateVeritces[i + 1], 10, 0.2f);
						finalVertices.insert(finalVertices.end(), subVertices.begin(), subVertices.end());
					}
				}
				else
					finalVertices = ConvertToVec3Vector(intermediateVeritces);

				splineDecoration.vertices = std::move(finalVertices);
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
				tryAddOrMoveAdjacentControlPoint();
			else
				movingAdjacentControlPoint = std::nullopt;

			if (keyboard.pressed[' '])
				lightning = !lightning;

			if (keyboard.pressed['L'])
				loop = !loop;

			if (keyboard.pressed['S'])
				splineAsPolyline = !splineAsPolyline;

			if (keyboard.pressed[0x26/*VK_UP*/] && complexity < 20)
				++complexity;

			if (keyboard.pressed[0x28/*VK_DOWN*/] && complexity > 1)
				--complexity;

			updateSpline();

			projectionHSize = std::clamp(projectionHSize + mouse.pressed.wheel * -5.0f, 5.0f, 100.0f);
		}

	private:
		struct ControlPoint
		{
			ComponentId decorationId;
			glm::vec2 pos;
		};

		glm::vec2 mousePos{};
		glm::vec2 cameraPos{};
		std::list<ControlPoint> controlPoints;
		std::optional<decltype(controlPoints)::iterator> movingControlPoint;
		std::optional<decltype(controlPoints)::iterator> movingAdjacentControlPoint;
		bool movingCamera = false;
		ComponentId splineDecorationId = 0;
		bool lightning = false;
		bool loop = false;
		bool splineAsPolyline = true;
		float projectionHSize = 10.0f;
		int complexity = 10;
	};

	SplineTest::SplineTest() :
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
