#include "raceEditor.hpp"

#include "startingLineEditing.hpp"
#include "splinesEditing.hpp"

#include <components/mouse.hpp>
#include <components/keyboard.hpp>
#include <components/decoration.hpp>
#include <components/screenInfo.hpp>
#include <components/camera.hpp>
#include <components/texture.hpp>
#include <components/animatedTexture.hpp>

#include <globals/components.hpp>

#include <tools/playersHandler.hpp>
#include <tools/shapes2D.hpp>

#include <glm/gtx/vector_angle.hpp>

#include <algorithm>
#include <optional>
#include <fstream>
#include <array>

namespace
{
	constexpr float cursorRadius = 0.25f;
}

namespace Levels
{
	void createSplines();

	class RaceEditor::Impl
	{
	public:
		RaceEditor::Impl():
			startingLineEditing(mousePos, oldMousePos, mouseDelta, zoomScale, [this]() { return !playersHandler && editMode == 1; }),
			splineEditing(mousePos, oldMousePos, mouseDelta, zoomScale, [this]() { return !playersHandler && editMode == 2; })
		{
		}

		void setup()
		{
			auto& staticDecoration = Globals::Components().staticDecorations();
			auto& textures = Globals::Components().textures();

			staticDecoration.emplace(Shapes2D::CreateVerticesOfCircle({ 0.0f, 0.0f }, cursorRadius, 20));
			staticDecoration.last().modelMatrixF = [this]() {
				return glm::scale(glm::translate(glm::mat4{ 1.0f }, { mousePos, 0.0f }), glm::vec3(zoomScale));
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

			for (auto& flameAnimatedTextureForPlayer : flameAnimatedTextureForPlayers)
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

		void initPlayersHandler(bool startingLine)
		{
			playersHandler = Tools::PlayersHandler();
			playersHandler->setCamera(Tools::PlayersHandler::CameraParams().projectionHSizeMin([]() { return 10.0f; }).scalingFactor(0.7f));
			playersHandler->initPlayers(planeTextures, flameAnimatedTextureForPlayers, false,
				[&](unsigned playerId, unsigned numOfPlayers) {
					const auto startingLineEnds = startingLineEditing.getStartingLineEnds();
					if (!startingLine || startingLineEnds.empty())
						return glm::vec3(mousePos, 0.0f);

					const glm::vec2 startingLineEndsVector = startingLineEnds[1] - startingLineEnds[0];
					const float startingLineLength = glm::length(startingLineEndsVector);
					const float playerPositionOnStartingLine = startingLineLength * (playerId + 1) / (numOfPlayers + 1);
					const glm::vec2 playerPositionOnStartingLine2D = startingLineEnds[0] + startingLineEndsVector * playerPositionOnStartingLine / startingLineLength;
					const glm::vec2 ntv = glm::rotate(glm::normalize(startingLineEndsVector), -glm::half_pi<float>());
					return glm::vec3(playerPositionOnStartingLine2D + ntv * startingLineEditing.getStartingPositionLineDistance(),
						glm::orientedAngle({ -1.0f, 0.0f }, ntv));
				}, startingLine);
		}

		void step()
		{
			const auto& mouse = Globals::Components().mouse();
			const auto& keyboard = Globals::Components().keyboard();
			const auto& screenInfo = Globals::Components().screenInfo();
			const float screenRatio = screenInfo.getAspectRatio();

			oldMousePos = mousePos;

			auto& dynamicDecorations = Globals::Components().dynamicDecorations();

			if (playersHandler)
				playersHandler->controlStep();
			else
			{
				mousePos += mouse.getWorldSpaceDelta() * projectionHSize * 0.001f;
				mousePos.x = std::clamp(mousePos.x, -projectionHSize * screenRatio + cameraPos.x, projectionHSize * screenRatio + cameraPos.x);
				mousePos.y = std::clamp(mousePos.y, -projectionHSize + cameraPos.y, projectionHSize + cameraPos.y);
			}

			mouseDelta = mousePos - oldMousePos;

			auto resetView = [&]() {
				cameraPos = glm::vec2(0.0f);
			};

			auto generateCode = [&]() {
				std::ofstream fs("levels/race/generatedCode.hpp");

				fs << "#include <components/polyline.hpp>\n";
				fs << "\n";
				fs << "#include <globals/components.hpp>\n";
				fs << "\n";
				fs << "#include <tools/splines.hpp>\n";
				fs << "#include <tools/b2Helpers.hpp>\n";
				fs << "\n";
				fs << "#include <glm/vec2.hpp>\n";
				fs << "#include <glm/vec3.hpp>\n";
				fs << "#include <glm/vec4.hpp>\n";
				fs << "#include <glm/gtx/vector_angle.hpp>\n";
				fs << "\n";
				fs << "#include <unordered_set>\n";
				fs << "\n";
				fs << "namespace GeneratedCode\n";
				fs << "{\n";
				fs << "\n";
				startingLineEditing.generateCode(fs);
				splineEditing.generateCode(fs);
				fs << "}\n";
			};

			for (int i = 0; i < 10; ++i)
				if (keyboard.pressed['0' + i])
					editMode = i;

			if (keyboard.pressed['T'])
				if (playersHandler)
				{
					playersHandler = std::nullopt;
					setEditorCamera();
				}
				else
					initPlayersHandler(keyboard.pressing[/*VK_SHIFT*/0x10]);

			if (!keyboard.pressing[/*VK_SHIFT*/0x10] && !keyboard.pressing[/*VK_CONTROL*/0x11] && !keyboard.pressing[/*VK_SPACE*/' '])
			{
				projectionHSize = std::clamp(projectionHSize + mouse.pressed.wheel * -10.0f * zoomScale, 1.0f, 10000.0f);
				zoomScale = projectionHSize / 50.0f;
			}

			if (!playersHandler)
			{
				if (keyboard.pressed['R'])
					resetView();

				if (keyboard.pressed['G'])
					generateCode();

				switch (editMode)
				{
				case 1:
					startingLineEditing.edit(cameraMoving); break;
				case 2:
					splineEditing.edit(cameraMoving); break;
				}

				if (cameraMoving)
					cameraPos += mouseDelta;
			}

			startingLineEditing.update();
			splineEditing.update();
		}

	private:
		glm::vec2 mousePos{};
		glm::vec2 oldMousePos{};
		glm::vec2 mouseDelta{};

		glm::vec2 cameraPos{};

		bool cameraMoving = false;
		float projectionHSize = 50.0f;
		float zoomScale = 1.0f;

		std::array<ComponentId, 4> planeTextures{ 0 };
		ComponentId flameAnimationTexture = 0;

		std::array<ComponentId, 4> flameAnimatedTextureForPlayers{ 0 };

		std::optional<Tools::PlayersHandler> playersHandler;

		int editMode = 2;

		StartingLineEditing startingLineEditing;
		SplineEditing splineEditing;
	};

	RaceEditor::RaceEditor() :
		impl(std::make_unique<Impl>())
	{
		impl->setup();
		createSplines();
	}

	RaceEditor::~RaceEditor() = default;

	void RaceEditor::step()
	{
		impl->step();
	}
}
