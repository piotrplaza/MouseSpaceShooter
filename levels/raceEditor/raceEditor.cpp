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
#include <components/graphicsSettings.hpp>

#include <globals/components.hpp>

#include <tools/playersHandler.hpp>
#include <tools/Shapes2D.hpp>
#include <tools/particleSystemHelpers.hpp>
#include <tools/paramsFromFile.hpp>

#include <glm/gtx/vector_angle.hpp>

#include <algorithm>
#include <optional>
#include <fstream>
#include <array>
#include <sstream>

namespace
{
	static const char* paramsPath = "levels/raceEditor/params.txt";
}

namespace Levels
{
	class RaceEditor::Impl
	{
	public:
		RaceEditor::Impl():
			paramsFromFile(paramsPath),
			startingLineEditing(mousePos, oldMousePos, mouseDelta, zoomScale, scale, [this]() { return !playersHandler && editMode == 1; }, paramsFromFile),
			splineEditing(mousePos, oldMousePos, mouseDelta, zoomScale, scale, [this]() { return !playersHandler && editMode == 2; }, paramsFromFile),
			grappleEditing(mousePos, oldMousePos, mouseDelta, zoomScale, scale, [this]() { return !playersHandler && editMode == 3; }, paramsFromFile)
		{
		}

		void loadParams()
		{
			paramsFromFile.loadParam(editProjectionHSize, "defaults.editProjectionHSize", false);
			paramsFromFile.loadParam(gameProjectionHSizeMin, "defaults.gameProjectionHSizeMin", false);
			paramsFromFile.loadParam(cursorRadius, "editor.mouse.cursorRadius", false);
			paramsFromFile.loadParam(cursorColor, "editor.mouse.cursorColor", false);
			paramsFromFile.loadParam(cursorSensitivity, "editor.mouse.moveSensitivity", false);
			paramsFromFile.loadParam(wheelSensitivity, "editor.mouse.wheelSensitivity", false);
			paramsFromFile.loadParam(backgroundColor, "background.color", false);
			paramsFromFile.loadParam(backgroundImagePath, "background.imagePath", false);
			{
				size_t pos;
				while ((pos = backgroundImagePath.find('"')) != std::string::npos)
					backgroundImagePath.erase(pos, 1);
				backgroundImagePath = "textures/" + backgroundImagePath;
			}
			paramsFromFile.loadParam(musicPath, "audio.music", false);
			{
				size_t pos;
				while ((pos = musicPath.find('"')) != std::string::npos)
					musicPath.erase(pos, 1);
				musicPath = "\"audio/" + musicPath + "\"";
			}
		}

		void setup()
		{
			loadParams();

			auto& graphicsSettings = Globals::Components().graphicsSettings();

			graphicsSettings.backgroundColorF = glm::vec4(backgroundColor, 1.0f);
			graphicsSettings.cullFace = false;

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

			backgroundTexture = textures.size();
			textures.emplace(backgroundImagePath);
		}

		void postSetup()
		{
			createBackground();
			initCollisions();

			auto& appStateHandler = Globals::Components().appStateHandler();
			auto& staticDecoration = Globals::Components().staticDecorations();

			appStateHandler.exitF = [&keyboard = Globals::Components().keyboard()]() { return keyboard.pressed[/*VK_ESCAPE*/ 0x1B] && keyboard.pressing[/*VK_SHIFT*/ 0x10]; };

			auto& cursor = staticDecoration.emplace(Tools::Shapes2D::CreatePositionsOfDisc({ 0.0f, 0.0f }, cursorRadius, 20));
			cursor.colorF = cursorColor;
			cursor.modelMatrixF = [this]() {
				return glm::scale(glm::translate(glm::mat4{ 1.0f }, { mousePos, 0.0f }), glm::vec3(zoomScale));
			};
			cursor.renderLayer = RenderLayer::NearForeground;
			cursor.renderF = [&]() {
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
			camera.positionTransitionFactor = 10.0f;
			camera.projectionTransitionFactor = 10.0f;
		}

		void initPlayersHandler(bool startingLine)
		{
			playersHandler = Tools::PlayersHandler();
			playersHandler->setCamera(Tools::PlayersHandler::CameraParams().projectionHSizeMin([&]() { return gameProjectionHSizeMin; }).transitionFactor(2.0f).scalingFactor(0.9f).velocityFactor(1.0f));
			playersHandler->initPlayers(Tools::PlayersHandler::InitPlayerParams{}.planeTextures(planeTextures).flameTextures(flameAnimatedTextureForPlayers).gamepadForPlayer1(false).initLocationFunc(
				[&](unsigned playerId, unsigned numOfPlayers) {
					const auto startingLineEnds = startingLineEditing.getStartingLineEnds();
					if (!startingLine || startingLineEnds.empty())
						return glm::vec3(mousePos, 0.0f);

					const glm::vec2 startingLineEndsVector = (startingLineEnds[1] - startingLineEnds[0]) * scale;
					const float startingLineLength = glm::length(startingLineEndsVector);
					const float playerPositionOnStartingLine = startingLineLength * (playerId + 1) / (numOfPlayers + 1);
					const glm::vec2 playerPositionOnStartingLine2D = startingLineEnds[0] * scale + startingLineEndsVector * playerPositionOnStartingLine / startingLineLength;
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
				mousePos += mouse.getCartesianDelta() * editProjectionHSize * cursorSensitivity;
				mousePos.x = std::clamp(mousePos.x, -editProjectionHSize * screenRatio + cameraPos.x, editProjectionHSize * screenRatio + cameraPos.x);
				mousePos.y = std::clamp(mousePos.y, -editProjectionHSize + cameraPos.y, editProjectionHSize + cameraPos.y);
			}

			mouseDelta = mousePos - oldMousePos;

			auto resetView = [&]() {
				cameraPos = glm::vec2(0.0f);
			};

			if (keyboard.pressed['G'])
				generateCode();

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

			if (!keyboard.pressing[/*VK_SHIFT*/0x10] && !keyboard.pressing[/*VK_CONTROL*/0x11] && !keyboard.pressing[/*VK_MENU*/0x12] && !keyboard.pressing[' '])
			{
				if (playersHandler)
				{
					gameProjectionHSizeMin = std::clamp(gameProjectionHSizeMin + mouse.pressed.wheel * -5.0f, 1.0f, 10000.0f);
				}
				else
				{
					editProjectionHSize = std::clamp(editProjectionHSize + mouse.pressed.wheel * -10.0f * zoomScale, 1.0f, 10000.0f);
					zoomScale = editProjectionHSize / 50.0f;
				}
			}

			if (keyboard.pressing[' '] && mouse.pressed.wheel)
				scale = std::clamp(scale + mouse.pressed.wheel * 0.1f, 0.1f, 10.0f);

			if (!playersHandler)
			{
				if (keyboard.pressed['R'])
					resetView();

				switch (editMode)
				{
				case 1:
					startingLineEditing.edit(); break;
				case 2:
					splineEditing.edit(); break;
				case 3:
					grappleEditing.edit(); break;
				case 9:
					editBackground(); break;
				}

				if (mouse.pressing.rmb && !keyboard.pressing[/*VK_SHIFT*/ 0x10] && !keyboard.pressing[/*VK_CONTROL*/ 0x11] && !keyboard.pressing[/*VK_MENU*/ 0x12] && !keyboard.pressing[' '])
					cameraPos += mouseDelta;
			}
			else
			{
				if (keyboard.pressed['R'])
				{
					playersHandler = std::nullopt;
					initPlayersHandler(!keyboard.pressing[/*VK_SHIFT*/0x10]);
				}
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

		void createBackground()
		{
			const auto& textures = Globals::Components().staticTextures();
			auto& staticDecoration = Globals::Components().staticDecorations();

			backgroundImageAspectRatio = textures[backgroundTexture].loaded.getAspectRatio();
			backgroundImageScale = glm::vec2(editProjectionHSize * 2);

			auto& background = staticDecoration.emplace(Tools::Shapes2D::CreatePositionsOfRectangle({}, { 0.5f * backgroundImageAspectRatio, 0.5f }));
			background.texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
			background.modelMatrixF = [this]() {
				return glm::scale(glm::translate(glm::mat4{ 1.0f }, glm::vec3(backgroundImagePosition, 0.0f)), glm::vec3(backgroundImageScale * scale, 1.0f));
			};
			background.renderLayer = RenderLayer::Background;
			background.texture = CM::Texture(backgroundTexture, true);
		}

		void editBackground()
		{
			const auto& mouse = Globals::Components().mouse();
			const auto& keyboard = Globals::Components().keyboard();

			cameraMoving = mouse.pressing.rmb;

			if (mouse.pressing.lmb || (bool)mouse.pressed.wheel)
			{
				if (keyboard.pressing[/*VK_SHIFT*/ 0x10])
					backgroundImageScale += mouse.getCartesianDelta() * editProjectionHSize * cursorSensitivity + (float)mouse.pressed.wheel * wheelSensitivity;
				else
					backgroundImagePosition += mouse.getCartesianDelta() * editProjectionHSize * cursorSensitivity;
			}
		}

		void initCollisions()
		{
			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::actor, Globals::CollisionBits::polyline, [this](const auto& plane, const auto& polyline) {
				Globals::Components().deferredActions().emplace([&](auto) {
					auto& planeComponent = Tools::AccessComponent<CM::Plane>(plane);
					const auto& polylineComponent = Tools::AccessComponent<CM::Polyline>(polyline);

					if (!planeComponent.isEnabled())
						return false;

					destroyPlane(planeComponent);

					return false;
				});
			});
		}

	private:
		void generateCode() const
		{
			std::ofstream fs("levels/race/generatedCode.hpp");

			fs << "#include <components/polyline.hpp>\n";
			fs << "#include <components/decoration.hpp>\n";
			fs << "#include <components/texture.hpp>\n";
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
			fs << "constexpr static float scale = (float)" << scale << ";\n";
			fs << "constexpr static glm::vec4 backgroundColor = {(float)" << backgroundColor.r << ", (float)" << backgroundColor.g << ", (float)" << backgroundColor.b << ", (float)1};\n";
			fs << "constexpr static glm::vec2 backgroundImagePosition = {(float)" << backgroundImagePosition.x << ", (float)" << backgroundImagePosition.y << "};\n";
			fs << "constexpr static float backgroundImageAspectRatio = (float)" << backgroundImageAspectRatio << ";\n";
			fs << "constexpr static glm::vec2 backgroundImageScale = {(float)" << backgroundImageScale.x << ", (float)" << backgroundImageScale.y << "};\n";
			fs << "constexpr static float projectionHSizeMin = (float)" << gameProjectionHSizeMin << ";\n";
			fs << "constexpr static const char* musicPath = " << musicPath << ";\n";
			fs << "\n";
			fs << "inline void CreateBackground(ComponentId& backgroundTextureId, ComponentId& backgroundDecorationId)\n";
			fs << "{\n";
			if (!backgroundImagePath.empty())
			{
				fs << "	backgroundTextureId = Globals::Components().staticTextures().size();\n";
				fs << "	Globals::Components().staticTextures().emplace(\"" << backgroundImagePath << "\");\n";
				fs << "	auto& background = Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({}, { 0.5f * backgroundImageAspectRatio, 0.5f }));\n";
				fs << "	background.texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();\n";
				fs << "	background.modelMatrixF = glm::scale(glm::translate(glm::mat4{ 1.0f }, glm::vec3(backgroundImagePosition, 0.0f)), glm::vec3(backgroundImageScale * scale, 1.0f));\n";
				fs << "	background.renderLayer = RenderLayer::Background;\n";
				fs << "	background.texture = CM::Texture(backgroundTextureId, true);\n";
				fs << "	backgroundDecorationId = background.getComponentId();\n";
			}
			fs << "}\n";
			fs << "\n";

			startingLineEditing.generateCode(fs);
			splineEditing.generateCode(fs);
			grappleEditing.generateCode(fs);
			fs << "}\n";
		}

		Tools::ParamsFromFile paramsFromFile;

		glm::vec2 mousePos{};
		glm::vec2 oldMousePos{};
		glm::vec2 mouseDelta{};

		glm::vec2 cameraPos{};

		bool cameraMoving = false;
		float editProjectionHSize = 50.0f;
		float gameProjectionHSizeMin = 30.0f;
		float zoomScale = 1.0f;
		float cursorRadius = 0.25f;
		glm::vec4 cursorColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		float cursorSensitivity = 0.001f;
		float wheelSensitivity = 5.0f;
		float scale = 1.0f;
		glm::vec3 backgroundColor = { 0.0f, 0.0f, 0.0f };
		std::string backgroundImagePath;
		std::string musicPath;
		float backgroundImageAspectRatio = 1.0f;
		glm::vec2 backgroundImagePosition = { 0.0f, 0.0f };
		glm::vec2 backgroundImageScale = { 1.0f, 1.0f };

		std::array<CM::Texture, 4> planeTextures;
		ComponentId flameAnimationTexture = 0;
		ComponentId explosionTexture = 0;
		ComponentId backgroundTexture = 0;

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
