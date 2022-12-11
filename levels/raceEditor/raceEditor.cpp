#include "raceEditor.hpp"

#include <components/mouse.hpp>
#include <components/keyboard.hpp>
#include <components/decoration.hpp>
#include <components/screenInfo.hpp>
#include <components/camera.hpp>
#include <components/texture.hpp>
#include <components/animatedTexture.hpp>

#include <globals/components.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/glmHelpers.hpp>
#include <tools/splines.hpp>
#include <tools/utility.hpp>

#include <tools/playersHandler.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <optional>
#include <list>
#include <unordered_map>

#include <iostream>
using namespace std;

namespace
{
	constexpr glm::vec4 activeSplineColor(1.0f);
	constexpr glm::vec4 inactiveSplineColor(0.5f);
	constexpr glm::vec4 activeControlPointColor(0.0f, 1.0f, 0.0f, 1.0f);
	constexpr glm::vec4 inactiveControlPointColor(0.0f, 0.5f, 0.0f, 0.5f);

	constexpr float cursorRadius = 0.25f;
	constexpr float controlPointRadius = 0.5f;
}

namespace Levels
{
	class RaceEditor::Impl
	{
	public:
		void setup()
		{
			auto& staticDecoration = Globals::Components().staticDecorations();
			auto& textures = Globals::Components().textures();

			staticDecoration.emplace(Tools::CreateVerticesOfCircle({ 0.0f, 0.0f }, cursorRadius, 20));
			staticDecoration.last().modelMatrixF = [this]() {
				return glm::translate(glm::mat4{ 1.0f }, { mousePos, 0.0f });
			};
			staticDecoration.last().renderLayer = RenderLayer::NearForeground;
			staticDecoration.last().renderF = [&]()
			{
				return !playersHandler;
			};

			setEditorCamera();

			planeTextures[0] = textures.size();
			textures.emplace("textures/plane 1.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.6f, 1.8f);
			textures.last().minFilter = GL_LINEAR;

			planeTextures[1] = textures.size();
			textures.emplace("textures/alien ship 1.png");
			textures.last().translate = glm::vec2(-0.2f, 0.0f);
			textures.last().scale = glm::vec2(1.9f);
			textures.last().minFilter = GL_LINEAR;

			planeTextures[2] = textures.size();
			textures.emplace("textures/plane 2.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.8f, 1.8f);
			textures.last().minFilter = GL_LINEAR;

			planeTextures[3] = textures.size();
			textures.emplace("textures/alien ship 2.png");
			textures.last().translate = glm::vec2(0.0f, 0.0f);
			textures.last().scale = glm::vec2(1.45f, 1.4f);
			textures.last().minFilter = GL_LINEAR;

			flameAnimationTexture = textures.size();
			textures.emplace("textures/flame animation 1.jpg");
			textures.last().minFilter = GL_LINEAR;

			for (unsigned& flameAnimatedTextureForPlayer : flameAnimatedTextureForPlayers)
			{
				flameAnimatedTextureForPlayer = Globals::Components().animatedTextures().size();
				Globals::Components().animatedTextures().add({ flameAnimationTexture, { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
					AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal });
				Globals::Components().animatedTextures().last().start(true);
			}
		}

		void setEditorCamera() const
		{
			auto& camera = Globals::Components().camera();

			camera.targetProjectionHSizeF = [&]() {
				return projectionHSize;
			};
			camera.targetPositionF = [&]() {
				return cameraPos;
			};
			camera.positionTransitionFactor = 1.0f;
			camera.projectionTransitionFactor = 1.0f;
		}

		void initPlayersHandler()
		{
			playersHandler = Tools::PlayersHandler();
			playersHandler->setCamera(Tools::PlayersHandler::CameraParams().projectionHSizeMin([]() { return 10.0f; }).scalingFactor(0.7f));
			playersHandler->initPlayers(planeTextures, flameAnimatedTextureForPlayers, true,
				[this](auto, auto) {
					return glm::vec3(mousePos, 0.0f);
				});
		}

		void step()
		{
			const auto& mouse = Globals::Components().mouse();
			const auto& keyboard = Globals::Components().keyboard();
			const auto& screenInfo = Globals::Components().screenInfo();
			const float screenRatio = (float)screenInfo.windowSize.x / screenInfo.windowSize.y;
			const glm::vec2 oldMousePos = mousePos;

			auto& dynamicDecorations = Globals::Components().dynamicDecorations();

			if (playersHandler)
				playersHandler->controlStep();
			else
			{
				mousePos += mouse.getWorldSpaceDelta() * projectionHSize * 0.001f;
				mousePos.x = std::clamp(mousePos.x, -projectionHSize * screenRatio + cameraPos.x, projectionHSize * screenRatio + cameraPos.x);
				mousePos.y = std::clamp(mousePos.y, -projectionHSize + cameraPos.y, projectionHSize + cameraPos.y);
			}

			const glm::vec2 mouseDelta = mousePos - oldMousePos;

			auto addControlPoint = [&](std::optional<std::list<ControlPoint>::iterator> it = std::nullopt) {
				if (!it)
				{
					if (!activeSplineDecorationId)
						activeSplineDecorationId = dynamicDecorations.emplace().getComponentId();

					dynamicDecorations.last().colorF = [&, id = *activeSplineDecorationId]() {
						return (activeSplineDecorationId && id == *activeSplineDecorationId)
							? activeSplineColor 
							: inactiveSplineColor;
					};
					dynamicDecorations.last().drawMode = GL_LINE_STRIP;
					dynamicDecorations.last().renderLayer = RenderLayer::Foreground;
				}

				auto& controlPoints = splineDecorationIdToSplineDef[*activeSplineDecorationId].controlPoints;

				dynamicDecorations.emplace(Tools::CreateVerticesOfCircle({ 0.0f, 0.0f }, controlPointRadius, 20));
				dynamicDecorations.last().colorF = [&, id = *activeSplineDecorationId]() {
					return (activeSplineDecorationId && id == *activeSplineDecorationId)
						? activeControlPointColor
						: inactiveControlPointColor;
				};
				auto insertedIt = controlPoints.insert(it ? *it : controlPoints.begin(), { dynamicDecorations.last().getComponentId(), mousePos });
				dynamicDecorations.last().modelMatrixF = [&pos = insertedIt->pos]() {
					return glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
				};
				dynamicDecorations.last().renderF = [&]()
				{
					return !playersHandler;
				};

				return insertedIt;
			};

			auto controlPointAction = [&](std::function<void(ComponentId, SplineDef&, std::list<ControlPoint>::iterator)> action)
			{
				for (auto& [splineDecorationId, splineDef] : splineDecorationIdToSplineDef)
					for (auto it = splineDef.controlPoints.begin(); it != splineDef.controlPoints.end(); ++it)
						if (glm::distance(it->pos, oldMousePos) < controlPointRadius)
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

			auto removeControlPointOrMoveCamera = [&]() {
				if (movingCamera)
				{
					cameraPos += mouseDelta;
					return;
				}

				bool removed = false;
				controlPointAction([&](auto splineDecorationId, auto& splineDef, auto it) {
					if (movingControlPoint || movingAdjacentControlPoint)
						return;

					dynamicDecorations[it->decorationId].state = ComponentState::Outdated;
					splineDef.controlPoints.erase(it);
					if (splineDef.controlPoints.empty())
						splineDecorationIdToSplineDef.erase(splineDecorationId);
					removed = true;
					});

				if (!removed)
					movingCamera = true;
			};

			auto tryAddOrMoveAdjacentControlPoint = [&]() {
				if (!movingAdjacentControlPoint)
					controlPointAction([&](auto, auto, auto it) {
						movingAdjacentControlPoint = addControlPoint(keyboard.pressing[/*VK_SHIFT*/0x10]
							? std::next(it)
							: it);
						});

				if (movingAdjacentControlPoint)
					(*movingAdjacentControlPoint)->pos += mouseDelta;
			};

			auto tryUpdateSpline = [&]() {
				for (auto& [splineDecorationId, SplineDef] : splineDecorationIdToSplineDef)
				{
					const auto& splineDef = splineDecorationIdToSplineDef[splineDecorationId];
					auto& splineDecoration = Globals::Components().dynamicDecorations()[splineDecorationId];

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

							const float rD = 0.05f;
							auto& v = intermediateVeritces[i];

							if (splineDef.loop && i == 0)
							{
								const auto& prevV = intermediateVeritces[intermediateVeritces.size() - 2];
								const auto& nextV = intermediateVeritces[1];
								const float avgLength = (glm::distance(prevV, v) + glm::distance(v, nextV)) * 0.5f;
								v += glm::vec2(Tools::Random(-rD, rD), Tools::Random(-rD, rD)) * avgLength;
								continue;
							}

							if (splineDef.loop && i == intermediateVeritces.size() - 1)
							{
								v = intermediateVeritces.front();
								continue;
							}

							const auto& prevV = intermediateVeritces[i - 1];
							const auto& nextV = intermediateVeritces[i + 1];
							const float avgLength = (glm::distance(prevV, v) + glm::distance(v, nextV)) * 0.5f;
							v += glm::vec2(Tools::Random(-rD, rD), Tools::Random(-rD, rD)) * avgLength;
						}

						for (size_t i = 0; i < intermediateVeritces.size() - 1; ++i)
						{
							std::vector<glm::vec3> subVertices = Tools::CreateVerticesOfLightning(intermediateVeritces[i], intermediateVeritces[i + 1], 10, 0.2f);
							finalVertices.insert(finalVertices.end(), subVertices.begin(), subVertices.end());
						}
					}
					else
						finalVertices = convertToVec3Vector(intermediateVeritces);

					splineDecoration.vertices = std::move(finalVertices);
					splineDecoration.state = ComponentState::Changed;
				}
			};

			auto deactivate = [&]() {
				auto& splineDecoration = Globals::Components().dynamicDecorations()[*activeSplineDecorationId];
				activeSplineDecorationId = std::nullopt;
			};

			auto delete_ = [&]() {
				auto& dynamicDecorations = Globals::Components().dynamicDecorations();

				for (const auto& controlPoint : splineDecorationIdToSplineDef[*activeSplineDecorationId].controlPoints)
					dynamicDecorations[controlPoint.decorationId].state = ComponentState::Outdated;

				dynamicDecorations[*activeSplineDecorationId].state = ComponentState::Outdated;
				splineDecorationIdToSplineDef.erase(*activeSplineDecorationId);
				activeSplineDecorationId = std::nullopt;
			};

			if (keyboard.pressed['T'])
				if (playersHandler)
				{
					playersHandler = std::nullopt;
					setEditorCamera();
				}
				else
					initPlayersHandler();

			if (!playersHandler)
			{
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

				if (keyboard.pressed['G'])
					generateCode();

				if (activeSplineDecorationId)
				{
					auto& splineDef = splineDecorationIdToSplineDef[*activeSplineDecorationId];

					if (keyboard.pressed[' '])
						splineDef.lightning = !splineDef.lightning;

					if (keyboard.pressed['L'])
						splineDef.loop = !splineDef.loop;

					if (keyboard.pressed['D'])
						deactivate();

					if (keyboard.pressed[/*VK_DELETE*/0x2E])
						delete_();

					if (keyboard.pressed[/*VK_UP*/0x26] && splineDef.complexity < 20)
						++splineDef.complexity;

					if (keyboard.pressed[/*VK_DOWN*/0x28] && splineDef.complexity > 1)
						--splineDef.complexity;
				}

				projectionHSize = std::clamp(projectionHSize + mouse.pressed.wheel * -10.0f, 10.0f, 200.0f);
			}

			tryUpdateSpline();
		}

	private:
		void generateCode() const
		{

			auto generateSplineCode = [](const auto& splineDef)
			{

			};

			for (const auto& [splineDecorationId, splineDef] : splineDecorationIdToSplineDef)
				generateSplineCode(splineDef);
		}

		struct ControlPoint
		{
			ComponentId decorationId;
			glm::vec2 pos;
		};

		struct SplineDef
		{
			std::list<ControlPoint> controlPoints;
			bool lightning = false;
			bool loop = false;
			int complexity = 10;
		};

		glm::vec2 mousePos{};
		glm::vec2 cameraPos{};
		std::unordered_map<ComponentId, SplineDef> splineDecorationIdToSplineDef;
		std::optional<ComponentId> activeSplineDecorationId;
		std::optional<std::list<ControlPoint>::iterator> movingControlPoint;
		std::optional<std::list<ControlPoint>::iterator> movingAdjacentControlPoint;
		bool movingCamera = false;
		float projectionHSize = 50.0f;

		std::array<unsigned, 4> planeTextures{ 0 };
		unsigned flameAnimationTexture = 0;

		std::array<unsigned, 4> flameAnimatedTextureForPlayers{ 0 };

		std::optional<Tools::PlayersHandler> playersHandler;
	};

	RaceEditor::RaceEditor() :
		impl(std::make_unique<Impl>())
	{
		impl->setup();
	}

	RaceEditor::~RaceEditor() = default;

	void RaceEditor::step()
	{
		impl->step();
	}
}
