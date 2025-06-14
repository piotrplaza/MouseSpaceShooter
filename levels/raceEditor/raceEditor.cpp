#include "raceEditor.hpp"

#include "startingLineEditing.hpp"
#include "splinesEditing.hpp"
#include "grappleEditing.hpp"

#include <components/mouse.hpp>
#include <components/keyboard.hpp>
#include <components/decoration.hpp>
#include <components/systemInfo.hpp>
#include <components/camera2D.hpp>
#include <components/texture.hpp>
#include <components/animatedTexture.hpp>
#include <components/appStateHandler.hpp>
#include <components/plane.hpp>
#include <components/collisionHandler.hpp>
#include <components/deferredAction.hpp>

#include <globals/components.hpp>

#include <tools/playersHandler.hpp>
#include <tools/Shapes2D.hpp>
#include <tools/particleSystemHelpers.hpp>

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
			splineEditing(mousePos, oldMousePos, mouseDelta, zoomScale, [this]() { return !playersHandler && editMode == 2; }),
			grappleEditing(mousePos, oldMousePos, mouseDelta, zoomScale, [this]() { return !playersHandler && editMode == 3; })
		{
		}

		void setup()
		{
			auto& textures = Globals::Components().staticTextures();

			planeTextures[0] = textures.emplace("textures/plane 1.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.6f, 1.8f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			planeTextures[1] = textures.emplace("textures/alien ship 1.png");
			textures.last().translate = glm::vec2(-0.2f, 0.0f);
			textures.last().scale = glm::vec2(1.9f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			planeTextures[2] = textures.emplace("textures/plane 2.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.8f, 1.8f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			planeTextures[3] = textures.emplace("textures/alien ship 2.png");
			textures.last().translate = glm::vec2(0.0f, 0.0f);
			textures.last().scale = glm::vec2(1.45f, 1.4f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			flameAnimationTexture = textures.size();
			textures.emplace("textures/flame animation 1_1.jpg");
			textures.last().minFilter = GL_LINEAR;

			explosionTexture = textures.size();
			textures.emplace("textures/explosion.png");

			initCollisions();
		}

		void postSetup()
		{
			auto& appStateHandler = Globals::Components().appStateHandler();
			auto& staticDecoration = Globals::Components().staticDecorations();

			appStateHandler.exitF = [&keyboard = Globals::Components().keyboard()]() { return keyboard.pressed[/*VK_ESCAPE*/ 0x1B] && keyboard.pressing[/*VK_SHIFT*/ 0x10]; };

			staticDecoration.emplace(Tools::Shapes2D::CreatePositionsOfCircle({ 0.0f, 0.0f }, cursorRadius, 20));
			staticDecoration.last().modelMatrixF = [this]() {
				return glm::scale(glm::translate(glm::mat4{ 1.0f }, { mousePos, 0.0f }), glm::vec3(zoomScale));
			};
			staticDecoration.last().renderLayer = RenderLayer::NearForeground;
			staticDecoration.last().renderF = [&]() {
				return !playersHandler;
			};

			setEditorCamera();

			for (auto& flameAnimatedTextureForPlayer : flameAnimatedTextureForPlayers)
			{
				flameAnimatedTextureForPlayer = Globals::Components().staticAnimatedTextures().add({ CM::Texture(flameAnimationTexture, true), { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 115 }, 0.02f, 32, 0,
					AnimationData::Direction::Backward, AnimationData::Mode::Repeat, AnimationData::TextureLayout::Horizontal });
				Globals::Components().staticAnimatedTextures().last().start(true);
			}
		}

		void setEditorCamera() const
		{
			auto& camera = Globals::Components().camera2D();
			camera.targetPositionAndProjectionHSizeF = [&]() {
				return glm::vec3(cameraPos, editProjectionHSize);
			};
		}

		void initPlayersHandler(bool startingLine)
		{
			playersHandler = Tools::PlayersHandler();
			playersHandler->setCamera(Tools::PlayersHandler::CameraParams{}.projectionHSizeMin([&]() { return testProjectionHSizeMin; }).scalingFactor(0.7f));
			playersHandler->initPlayers(Tools::PlayersHandler::InitPlayerParams{}.planeTextures(planeTextures).flameTextures(flameAnimatedTextureForPlayers).gamepadForPlayer1(false).initLocationFunc(
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
				}).centerToFront(startingLine));
		}

		void step()
		{
			const auto& mouse = Globals::Components().mouse();
			const auto& keyboard = Globals::Components().keyboard();
			const auto& screenInfo = Globals::Components().systemInfo().screen;
			const float screenRatio = screenInfo.getAspectRatio();

			oldMousePos = mousePos;

			auto& dynamicDecorations = Globals::Components().decorations();

			if (playersHandler)
				playersHandler->controlStep();
			else
			{
				mousePos += mouse.getCartesianDelta() * editProjectionHSize * 0.001f;
				mousePos.x = std::clamp(mousePos.x, -editProjectionHSize * screenRatio + cameraPos.x, editProjectionHSize * screenRatio + cameraPos.x);
				mousePos.y = std::clamp(mousePos.y, -editProjectionHSize + cameraPos.y, editProjectionHSize + cameraPos.y);
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
				fs << "#include <tools/Shapes2D.hpp>\n";
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
				grappleEditing.generateCode(fs);
				fs << "}\n";
			};

			for (int i = 0; i < 10; ++i)
				if (keyboard.pressed['0' + i])
				{
					playersHandler = std::nullopt;
					setEditorCamera();
					editMode = i;
				}

			if (keyboard.pressed['T'])
				if (playersHandler)
				{
					playersHandler = std::nullopt;
					setEditorCamera();
				}
				else
					initPlayersHandler(!keyboard.pressing[/*VK_SHIFT*/0x10]);

			if (!keyboard.pressing[/*VK_SHIFT*/0x10] && !keyboard.pressing[/*VK_CONTROL*/0x11] && !keyboard.pressing[/*VK_SPACE*/' '])
			{
				if (playersHandler)
				{
					testProjectionHSizeMin = std::clamp(testProjectionHSizeMin + mouse.pressed.wheel * -2.0f, 1.0f, 10000.0f);
				}
				else
				{
					editProjectionHSize = std::clamp(editProjectionHSize + mouse.pressed.wheel * -10.0f * zoomScale, 1.0f, 10000.0f);
					zoomScale = editProjectionHSize / 50.0f;
				}
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
				case 3:
					grappleEditing.edit(cameraMoving); break;
				}

				if (cameraMoving)
					cameraPos += mouseDelta;
			}

			startingLineEditing.update();
			splineEditing.update();
			grappleEditing.update();
		}

		void destroyPlane(Components::Plane& plane)
		{
			Tools::CreateExplosion(Tools::ExplosionParams().center(plane.getOrigin2D()).sourceVelocity(plane.getVelocity()).
				initExplosionVelocityRandomMinFactor(0.2f).explosionTexture(CM::Texture(explosionTexture, true)));
			plane.setEnabled(false);
		}

		void initCollisions()
		{
			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::actor, Globals::CollisionBits::polyline, [this](const auto& plane, const auto& polyline) {
				Globals::Components().deferredActions().emplace([&](auto) {
					auto& planeComponent = Tools::AccessComponent<CM::Plane>(plane);
					const auto& polylineComponent = Tools::AccessComponent<CM::Polyline>(polyline);

					destroyPlane(planeComponent);

					return false;
				});
			});
		}

	private:
		glm::vec2 mousePos{};
		glm::vec2 oldMousePos{};
		glm::vec2 mouseDelta{};

		glm::vec2 cameraPos{};

		bool cameraMoving = false;
		float editProjectionHSize = 50.0f;
		float testProjectionHSizeMin = 10.0f;
		float zoomScale = 1.0f;

		std::array<CM::Texture, 4> planeTextures;
		ComponentId flameAnimationTexture = 0;
		ComponentId explosionTexture = 0;

		std::array<CM::AnimatedTexture, 4> flameAnimatedTextureForPlayers;

		std::optional<Tools::PlayersHandler> playersHandler;

		int editMode = 1;

		StartingLineEditing startingLineEditing;
		SplineEditing splineEditing;
		GrappleEditing grappleEditing;
	};

	RaceEditor::RaceEditor() :
		impl(std::make_unique<Impl>())
	{
		impl->setup();
		createSplines();
	}

	RaceEditor::~RaceEditor() = default;

	void RaceEditor::postSetup()
	{
		impl->postSetup();
	}

	void RaceEditor::step()
	{
		impl->step();
	}
}
