#include "snakeCube.hpp"

#include <components/decoration.hpp>
#include <components/physics.hpp>
#include <components/graphicsSettings.hpp>
#include <components/camera3D.hpp>
#include <components/texture.hpp>
#include <components/functor.hpp>
#include <components/keyboard.hpp>
#include <components/light3D.hpp>
#include <components/music.hpp>
#include <components/soundBuffer.hpp>
#include <components/sound.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <tools/shapes3D.hpp>
#include <tools/colorBufferEditor.hpp>
#include <tools/glmHelpers.hpp>
#include <tools/gameHelpers.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <array>
#include <list>
#include <unordered_set>
#include <future>
#include <execution>

#include <iostream>

//#define TEST

namespace
{
	constexpr int boardSize = 21;
	constexpr float moveDuration = 0.1f;
	constexpr int lenghtening = 80;
	
	constexpr glm::vec4 clearColor = { 0.0f, 0.1f, 0.15f, 1.0f };
	constexpr glm::vec4 snakeHeadColor = { 0.0f, 0.8f, 0.6f, 1.0f };
	constexpr glm::vec4 snakeEatingHeadColor = { 0.0f, 1.0f, 0.8f, 1.0f };
	constexpr glm::vec4 snakeDeadHeadColor = { 0.8f, 0.0f, 0.0f, 1.0f };
	constexpr glm::vec4 snakeHeadSphereColor = { 0.4f, 0.0f, 0.0f, 1.0f };
	constexpr glm::vec4 snakeTailColor = { 0.0f, 0.6f, 0.0f, 1.0f };
	constexpr glm::vec4 snakeFoodColor = { 0.0f, 0.8f, 0.0f, 1.0f };
	constexpr glm::vec4 foodColor = { 0.0f, 1.0f, 0.0f, 1.0f };
	constexpr glm::vec4 wiredCubeColor = { 0.0f, 1.0f, 1.0f, 1.0f };
#ifndef TEST
	constexpr glm::vec4 cubeColor = { 0.0f, 0.0f, 0.0f, 0.0f };
#else
	constexpr glm::vec4 cubeColor = { 0.0f, 0.0f, 0.0f, 1.0f };
#endif
	constexpr float cubeHSize = 0.5f;
	constexpr float cameraDistance = 2.0f;

	constexpr float growingSoundVolume = 0.3f;

	constexpr int numOfCrosses = 10000;
	constexpr int numOfLights = 16;
	constexpr float crossesScale = 0.03f;
}

namespace Levels
{
	class SnakeCube::Impl
	{
	public:
		void setup()
		{
			{
				auto& graphicsSettings = Globals::Components().graphicsSettings();

				graphicsSettings.clearColor = clearColor;
				graphicsSettings.cullFace = false;
				graphicsSettings.lineWidth = 1.0f;
			}

			Tools::CreateJuliaBackground([this, cOffset = glm::vec2(0.0f)]() mutable {
				const auto& physics = Globals::Components().physics();
				const float step = 0.0001f * physics.frameDuration * (snakeHead->second.type != SnakeNode::Type::DeadHead);
				cOffset += glm::vec2(snakeDirection == SnakeDirection::Left ? -step : snakeDirection == SnakeDirection::Right ? step : 0.0f,
					snakeDirection == SnakeDirection::Down ? -step : snakeDirection == SnakeDirection::Up ? step : 0.0f);
				return cOffset;
			});

			{
				auto& staticTextures = Globals::Components().staticTextures();

				marbleTexture = staticTextures.emplace("textures/green marble.jpg").getComponentId();
				staticTextures.last().wrapMode = GL_MIRRORED_REPEAT;
			}
#ifdef TEST
			std::array<CM::StaticTexture, 6> testCubeTextures;
#endif
			for (size_t i = 0; i < 6; ++i)
			{
#ifdef TEST
				auto& staticTextures = Globals::Components().staticTextures();
				testCubeTextures[i] = &staticTextures.emplace(TextureData(TextureFile("textures/test/" + std::to_string(i) + ".png", 4)));
#endif
				auto& dynamicTextures = Globals::Components().dynamicTextures();
				auto& cubeTexture = cubeTextures[i];

				cubeTexture = &dynamicTextures.emplace(TextureData(std::vector<glm::vec4>(boardSize * boardSize, glm::vec4(0.0f)), glm::ivec2(boardSize)));
				cubeTexture.component->magFilter = GL_NEAREST;
				cubeTexture.component->wrapMode = GL_CLAMP_TO_EDGE;
			}

			{
				auto& dynamicDecorations = Globals::Components().dynamicDecorations();
				auto& instancedCrosses = dynamicDecorations.emplace();

				Shapes3D::AddCross(instancedCrosses, { 0.1f, 0.5f, 0.1f }, { 0.35f, 0.1f, 0.1f }, 0.15f, [](auto, glm::vec3 p) { return glm::vec2(p.x + p.z, p.y + p.z); }, glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)));
				instancedCrosses.modelMatrixF = [&]() { return glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)); };
				instancedCrosses.params3D->ambient(0.4f).diffuse(0.8f).specular(0.8f).specularMaterialColorFactor(0.2f).lightModelEnabled(true).gpuSideInstancedNormalTransforms(true);
				instancedCrosses.texture = CM::StaticTexture(marbleTexture);
				instancedCrosses.bufferDataUsage = GL_DYNAMIC_DRAW;
				instancedCrosses.instancing.emplace().init(numOfCrosses, glm::mat4(1.0f));
				instancedCrosses.renderLayer = RenderLayer::NearBackground;
				crossesId = instancedCrosses.getComponentId();

				{
					auto& lights3D = Globals::Components().lights3D();
					auto& staticDecorations = Globals::Components().staticDecorations();
					auto& physics = Globals::Components().physics();

					for (unsigned i = 0; i < numOfLights; ++i)
					{
						auto& light = lights3D.emplace(glm::vec3(0.0f), glm::vec3(1.0f), 0.3f / crossesScale, 0.0f);
						light.stepF = [&]() { light.setEnable(instancedCrosses.isEnabled()); };

						auto& lightDecoration = staticDecorations.emplace();
						Shapes3D::AddSphere(lightDecoration, 0.2f * crossesScale, 2, 3);
						lightDecoration.colorF = [&]() { return glm::vec4(light.color, 1.0f) + (Globals::Components().graphicsSettings().clearColor) * light.clearColorFactor; };
						lightDecoration.params3D->lightModelEnabled(false);
						lightDecoration.modelMatrixF = [&]() { return glm::rotate(glm::translate(glm::mat4(1.0f), light.position), physics.simulationDuration * 4.0f, { 1.0f, 1.0f, 1.0f }); };
						lightDecoration.stepF = [&, &lightSphere = lightDecoration]() { lightSphere.setEnable(instancedCrosses.isEnabled()); };
						lightDecoration.renderLayer = RenderLayer::NearBackground;
					}
				}
			}

			{
				auto decorations = Shapes3D::CreateCuboid(Globals::Components().staticDecorations(), cubeTextures, glm::vec3(cubeHSize));
				for (auto* decoration : decorations)
					decoration->params3D->lightModelEnabled(false);
			}

			auto& wiredCuboid = Globals::Components().staticDecorations().emplace();
			Shapes3D::AddWiredCuboid(wiredCuboid, glm::vec3(cubeHSize - 0.001f), wiredCubeColor);
			wiredCuboid.params3D->lightModelEnabled(false);
#ifdef TEST
			auto cuboidWalls = Shapes3D::CreateCuboid(Globals::Components().staticDecorations(), testCubeTextures, glm::vec3(cubeHSize) + 0.001f);
			for (auto* wall : cuboidWalls)
				wall->colorF = []() { return glm::vec4(0.2f); };
#endif
			auto snakeHeadSphere = Globals::Components().staticDecorations().emplace();
			Shapes3D::AddSphere(snakeHeadSphere, 0.2f / boardSize, 20, 20, nullptr, false);
			snakeHeadSphere.colorF = []() { return snakeHeadSphereColor; };
			snakeHeadSphere.modelMatrixF = [&]() { return glm::translate(glm::mat4(1.0f), cubeCoordToPos(snakeHead->first)); };
			snakeHeadSphere.params3D->lightModelEnabled(false);

			Globals::Components().stepSetups().emplace([this]() {
				createEditors(); 
				gameplayInit();
				return false; 
			});

			auto& musics = Globals::Components().musics();
			musics.emplace("audio/Ghosthack-Ambient Beds_Granular Dreams_Fm 65Bpm (WET).ogg", 1.0f).play();

			auto& soundsBuffers = Globals::Components().soundsBuffers();
			eatingSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Synth - Pluck_C.wav").getComponentId();
			growingSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Synth - Choatic_C.wav").getComponentId();
			deadSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Scrape - Horror_C.wav").getComponentId();

			auto& sounds = Globals::Components().sounds();
			growingSound = sounds.emplace(growingSoundBuffer).setVolume(0.0f).setLoop(true).play().getComponentId();
			deadSound = sounds.emplace(deadSoundBuffer).setVolume(0.0f).setLoop(true).play().getComponentId();
		}

		void step()
		{
			{
				auto& crosses = Globals::Components().dynamicDecorations()[crossesId];
				const auto& keyboard = Globals::Components().keyboard();

				if (keyboard.pressed['C'])
				{
					crosses.setEnable(!crosses.isEnabled());
				}

				if (crosses.isEnabled())
				{
					const float transformSpeed = 0.00001f;
					const float transformBaseStep = 0.001f;

					const auto& physics = Globals::Components().physics();
					auto& transforms = crosses.instancing->transforms_;

					if (keyboard.pressed[0xBB/*VK_OEM_PLUS*/])
						transformBase += transformBaseStep;
					if (keyboard.pressed[0xBD/*VK_OEM_MINUS*/])
						transformBase -= transformBaseStep;

					auto transform = [this, simulationDuration = physics.simulationDuration](auto i) {
						return glm::scale(glm::mat4(1.0f), glm::vec3(crossesScale))
							* glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0.0f, 1.0f, 0.0f })
							* glm::rotate(glm::mat4(1.0f), i * glm::pi<float>() * 0.001f, { 1.0f, 0.0f, 0.0f })
							* glm::rotate(glm::mat4(1.0f), i * glm::pi<float>() * 0.03f, { 0.0f, 1.0f, 0.0f })
							* glm::rotate(glm::mat4(1.0f), i * glm::pi<float>() * (transformBase - simulationDuration * transformSpeed), { 0.0f, 0.0f, 1.0f })
							* glm::translate(glm::mat4(1.0f), { i * 0.0005f, i * 0.0007f, i * 0.0009f });
						};
#if 1
					if (transformFuture.valid())
					{
						transformFuture.get();
						crosses.state = ComponentState::Changed;
					}

					transformFuture = std::async(std::launch::async, [=, simulationDuration = physics.simulationDuration, &transforms]() {
						Tools::ItToId itToId(transforms.size());
						std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), [=, &transforms](const auto i) {
							transforms[i] = transform(i);
							});
						});
#else
					Tools::ItToId itToId(transforms.size());
					std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), [&](const auto i) {
						transforms[i] = transform(i);
						});
					crosses.state = ComponentState::Changed;
#endif
					{
						int i = 0;
						for (auto& light : Globals::Components().lights3D())
						{
							const float rotationSpeed = (-0.1f - (i * 0.03f)) * (i % 2 * 2.0f - 1.0f) * 100.0f;
							const float radius = 1.0f + i * 5.0f;
							const glm::vec3 changeColorSpeed(0.1f, 0.6f, 0.3f);
							const float scaledSimulationDuration = physics.simulationDuration / (float)numOfLights;
							const auto colorDriver = changeColorSpeed * rotationSpeed * scaledSimulationDuration;

							light.position = glm::vec3(
								glm::cos(scaledSimulationDuration * rotationSpeed),
								glm::cos(scaledSimulationDuration * rotationSpeed * 0.3f),
								glm::sin(scaledSimulationDuration * rotationSpeed * 0.6f))
								* crossesScale / (float)numOfLights * radius;

							light.color = (glm::vec3(glm::cos(colorDriver.r), glm::cos(colorDriver.g), glm::cos(colorDriver.b)) + 1.0f) / 2.0f + lightColorOffset;
							if (snakeHead->second.type == SnakeNode::Type::DeadHead)
								lightColorOffset += glm::vec3(physics.frameDuration * 0.02f, glm::vec2(physics.frameDuration) * -0.02f);
							//light.color = glm::vec3(1.0f);
							++i;
						}
					}
				}
			}

			controlsStep();

			if (snakeHead->second.type == SnakeNode::Type::DeadHead)
			{
				cameraStep();
				return;
			}

			const auto& physics = Globals::Components().physics();
			moveTime += physics.frameDuration;

			if (moveTime < moveDuration)
			{
				cameraStep();
				return;
			}

			moveTime -= moveDuration;

			gameplayStep();
			cameraStep();
		}

	private:
		enum class CubeFace { Front, Back, Left, Right, Top, Bottom };
		enum class SnakeDirection { Up, Down, Left, Right };

		struct SnakeNode;
		using SnakeNodes = std::unordered_map<glm::ivec3, SnakeNode>;

		struct SnakeNode
		{
			enum class Type { Head, DeadHead, EatingHead, Tail, Food } type{};
			SnakeNodes::iterator prev;
			SnakeNodes::iterator next;
		};

		void createEditors()
		{
			for (size_t i = 0; i < 6; ++i)
			{
				auto& texture = cubeTextures[i];
				auto& loadedTextureData = std::get<TextureData>(texture.component->source).loaded;
				auto& buffer = std::get<std::vector<glm::vec4>>(loadedTextureData.data);
				cubeEditors[i] = std::make_unique<Tools::ColorBufferEditor<glm::vec4>>(buffer, glm::vec2(boardSize));
			}
		}

		void gameplayInit()
		{
			for (size_t i = 0; i < 6; ++i)
			{
				cubeEditors[i]->clear(cubeColor);
				cubeTextures[i].component->state = ComponentState::Changed;
			}

			moveTime = 0.0f;

			snakeNodes.clear();
			snakeNodes.reserve(6 * boardSize * boardSize);
			snakeNodes[glm::ivec3(cubeEditors[0]->getRes() / 2, 0)] = { SnakeNode::Type::EatingHead, snakeNodes.end(), snakeNodes.end() };
			snakeHead = snakeEnd = snakeNodes.begin();

			snakeDirection = SnakeDirection::Up;
			lastSnakeStep = { 0, 0 };
			up = { 0, 1 };
			targetCameraUp = { 0, 1, 0 };
			lenghteningLeft = lenghtening;
			foodPos = std::nullopt;
			score = 0;
			lightColorOffset = { 0.0f, 0.0f, 0.0f };

			freeSpace.clear();
			for (int z = 0; z < 6; ++z)
				for (int y = 0; y < boardSize; ++y)
					for (int x = 0; x < boardSize; ++x)
						freeSpace.insert({ x, y, z });
			freeSpace.erase(snakeHead->first);

			Globals::Components().sounds()[growingSound].setVolume(growingSoundVolume);
			Globals::Components().sounds()[deadSound].setVolume(0.0f);

			cameraSetup();
			spawnFood();
			drawSnake();
			drawFood();
		}

		void gameplayStep()
		{
			const auto erasedEndPos = [&]() -> std::optional<glm::ivec3> {
				if (lenghteningLeft > 0)
				{
					--lenghteningLeft;
					return std::nullopt;
				}
				if (snakeEnd->second.type == SnakeNode::Type::Food)
				{
					lenghteningLeft = lenghtening - 1;
					return std::nullopt;
				}

				auto pos = snakeEnd->first;
				auto newEnd = snakeEnd->second.next;
				freeSpace.insert(pos);
				snakeNodes.erase(snakeEnd);
				snakeEnd = newEnd;
				snakeEnd->second.prev = snakeNodes.end();
				return pos;
			}();

			Globals::Components().sounds()[growingSound].setVolume((float)lenghteningLeft / lenghtening * growingSoundVolume);

			if (!erasedEndPos && lenghteningLeft == 0)
				snakeEnd->second.type = SnakeNode::Type::Tail;

			snakeHead->second.type = snakeHead->second.type == SnakeNode::Type::EatingHead
				? SnakeNode::Type::Food
				: SnakeNode::Type::Tail;

			lastSnakeStep = snakeStep(snakeDirection);
			glm::ivec3 nextPos = snakeHead->first + glm::ivec3(lastSnakeStep, 0);
			handleBorders(nextPos);

			auto it = snakeNodes.find(nextPos);
			if (it == snakeNodes.end())
			{
				const bool eating = foodPos && *foodPos == nextPos;
				if (eating)
				{
					foodPos = std::nullopt;
					Tools::CreateAndPlaySound(eatingSoundBuffer).setVolume(0.6f);
					std::cout << ++score << std::endl;
				}
				auto prevHead = snakeHead;
				snakeHead = snakeNodes.emplace(nextPos, SnakeNode{ eating ? SnakeNode::Type::EatingHead : SnakeNode::Type::Head, prevHead, snakeNodes.end() }).first;
				prevHead->second.next = snakeHead;
				freeSpace.erase(nextPos);
			}
			else
			{
				it->second.prev = snakeHead;
				snakeHead = it;
				snakeHead->second.type = SnakeNode::Type::DeadHead;
				Globals::Components().sounds()[growingSound].setVolume(0.0f);
				Globals::Components().sounds()[deadSound].setVolume(0.3f);
			}

			if (!foodPos)
			{
				spawnFood();
				drawFood();
			}

			redrawSnake(erasedEndPos);
		}

		void spawnFood()
		{
			auto it = std::next(freeSpace.begin(), rand() % freeSpace.size());
			foodPos = *it;
			freeSpace.erase(it);
		}

		void handleBorders(glm::ivec3& pos)
		{
			auto changeCoordinates90CW = [&]() {
				up = Rotate90CW(up);
				lastSnakeStep = Rotate90CW(lastSnakeStep);
			};

			auto changeCoordinates90CCW = [&]() {
				up = Rotate90CCW(up);
				lastSnakeStep = Rotate90CCW(lastSnakeStep);
			};

			auto changeCoordinates180 = [&]() {
				up = Rotate180(up);
				lastSnakeStep = Rotate180(lastSnakeStep);
			};

			const glm::ivec2 prevUp = up;

			switch (pos[2])
			{
			case 0:
				if (pos.x < 0)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(targetCameraUp, 1));
					pos.x = boardSize - 1;
					pos.z = 2;
				}
				else if (pos.x >= boardSize)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(targetCameraUp, 1));
					pos.x = 0;
					pos.z = 3;
				}
				else if (pos.y < 0)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(targetCameraUp, 1));
					pos.y = boardSize - 1;
					pos.z = 4;
				}
				else if (pos.y >= boardSize)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(targetCameraUp, 1));
					pos.y = 0;
					pos.z = 5;
				}
				break;
			case 1:
				if (pos.x < 0)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(targetCameraUp, 1));
					pos.x = boardSize - 1;
					pos.z = 3;
				}
				else if (pos.x >= boardSize)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(targetCameraUp, 1));
					pos.x = 0;
					pos.z = 2;
				}
				else if (pos.y < 0)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(targetCameraUp, 1));
					changeCoordinates180();
					pos = { boardSize - 1 - pos.x, 0, 4 };
				}
				else if (pos.y >= boardSize)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(targetCameraUp, 1));
					changeCoordinates180();
					pos = { boardSize - 1 - pos.x, boardSize - 1, 5 };
				}
				break;
			case 2:
				if (pos.x < 0)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(targetCameraUp, 1));
					pos.x = boardSize - 1;
					pos.z = 1;
				}
				else if (pos.x >= boardSize)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(targetCameraUp, 1));
					pos.x = 0;
					pos.z = 0;
				}
				else if (pos.y < 0)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(targetCameraUp, 1));
					changeCoordinates90CCW();
					pos = { 0, pos.x, 4 };
				}
				else if (pos.y >= boardSize)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(targetCameraUp, 1));
					changeCoordinates90CW();
					pos = { 0, boardSize - 1 - pos.x, 5 };
				}
				break;
			case 3:
				if (pos.x < 0)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(targetCameraUp, 1));
					pos.x = boardSize - 1;
					pos.z = 0;
				}
				else if (pos.x >= boardSize)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(targetCameraUp, 1));
					pos.x = 0;
					pos.z = 1;
				}
				else if (pos.y < 0)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(targetCameraUp, 1));
					changeCoordinates90CW();
					pos = { boardSize - 1, boardSize - 1 - pos.x, 4 };
				}
				else if (pos.y >= boardSize)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(targetCameraUp, 1));
					changeCoordinates90CCW();
					pos = { boardSize - 1, pos.x, 5 };
				}
				break;
			case 4:
				if (pos.x < 0)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(targetCameraUp, 1));
					changeCoordinates90CW();
					pos = { pos.y, 0, 2 };
				}
				else if (pos.x >= boardSize)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(targetCameraUp, 1));
					changeCoordinates90CCW();
					pos = { boardSize - 1 - pos.y, 0, 3 };
				}
				else if (pos.y < 0)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(targetCameraUp, 1));
					changeCoordinates180();
					pos = { boardSize - 1 - pos.x, 0, 1 };
				}
				else if (pos.y >= boardSize)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(targetCameraUp, 1));
					pos.y = 0;
					pos.z = 0;
				}
				break;
			case 5:
				if (pos.x < 0)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(targetCameraUp, 1));
					changeCoordinates90CCW();
					pos = { boardSize - 1 - pos.y, boardSize - 1, 2 };
				}
				else if (pos.x >= boardSize)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(targetCameraUp, 1));
					changeCoordinates90CW();
					pos = { pos.y, boardSize - 1, 3 };	
				}
				else if (pos.y < 0)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(targetCameraUp, 1));
					pos.y = boardSize - 1;
					pos.z = 0;
				}
				else if (pos.y >= boardSize)
				{
					targetCameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(targetCameraUp, 1));
					changeCoordinates180();
					pos = { boardSize - 1 - pos.x, boardSize - 1, 1 };
				}
				break;
			}
		}

		void drawSnake()
		{
			for (const auto& [pos, node] : snakeNodes)
			{
				cubeEditors[pos[2]]->putColor(pos, snakeNodeColors.at(node.type));
				cubeTextures[pos[2]].component->state = ComponentState::Changed;
			}
		}

		void redrawSnake(const std::optional<glm::ivec3>& erasedEndPos)
		{
			if (erasedEndPos)
			{
				const auto& pos = *erasedEndPos;
				cubeEditors[pos[2]]->putColor(pos, cubeColor);
				cubeTextures[pos[2]].component->state = ComponentState::Changed;
			}

			cubeEditors[snakeHead->first[2]]->putColor(snakeHead->first, snakeNodeColors.at(snakeHead->second.type));
			cubeTextures[snakeHead->first[2]].component->state = ComponentState::Changed;

			cubeEditors[snakeEnd->first[2]]->putColor(snakeEnd->first, snakeEnd->second.type == SnakeNode::Type::Food
				? glm::mix(snakeNodeColors.at(SnakeNode::Type::Tail), snakeNodeColors.at(SnakeNode::Type::Food), (float)((lenghteningLeft == 0 ? lenghtening : lenghteningLeft)) / lenghtening)
				: snakeNodeColors.at(snakeEnd->second.type));
			cubeTextures[snakeEnd->first[2]].component->state = ComponentState::Changed;

			auto snakeNeck = snakeHead->second.prev;
			if (snakeNeck != snakeNodes.end())
			{
				cubeEditors[snakeNeck->first[2]]->putColor(snakeNeck->first, snakeNodeColors.at(snakeNeck->second.type));
				cubeTextures[snakeNeck->first[2]].component->state = ComponentState::Changed;
			}
		}

		void drawFood()
		{
			if (foodPos)
			{
				cubeEditors[foodPos->z]->putColor(*foodPos, foodColor);
				cubeTextures[foodPos->z].component->state = ComponentState::Changed;
			}
		}

		void controlsStep()
		{
			for (const auto& [key, direction]: controls)
				if (Globals::Components().keyboard().pressing[key])
				{
					if (lastSnakeStep == snakeStep(direction) || lastSnakeStep == -snakeStep(direction))
						continue;
					snakeDirection = direction;
				}

			if (Globals::Components().keyboard().pressed['R'])
				gameplayInit();
		}

		void cameraSetup() const
		{
			auto& camera = Globals::Components().camera3D();

			camera.position = glm::vec3(0.0f, 0.0f, cameraDistance);
			camera.rotation = Components::Camera3D::LookAtRotation{};
		}

		void cameraStep() const
		{
			const float cameraPositionSpeed = 2.0f / (moveDuration * boardSize);
			const float cameraUpSpeed = 5.0f / (moveDuration * boardSize);

			const auto& physics = Globals::Components().physics();
			auto& camera = Globals::Components().camera3D();
			const glm::vec3 targetCameraPos = glm::normalize(cubeCoordToPos(snakeHead->first)) * cameraDistance;
			const glm::vec3 interpolatedCameraPos = glm::mix(glm::vec3(camera.position), glm::vec3(targetCameraPos),
				std::clamp(cameraPositionSpeed * physics.frameDuration, 0.0f, 1.0f));
			camera.position = interpolatedCameraPos;

			auto& cameraUp = std::get<Components::Camera3D::LookAtRotation>(camera.rotation).up;
			const glm::vec3 interpolatedCameraUp = glm::normalize(glm::mix(glm::vec3(cameraUp), glm::vec3(targetCameraUp),
				std::clamp(cameraUpSpeed * physics.frameDuration, 0.0f, 1.0f)));
			cameraUp = interpolatedCameraUp;
		}

		glm::ivec2 snakeStep(SnakeDirection direction) const
		{
			switch (direction)
			{
			case SnakeDirection::Up:
				return up;
			case SnakeDirection::Down:
				return Rotate180(up);
			case SnakeDirection::Left:
				return Rotate90CCW(up);
			case SnakeDirection::Right:
				return Rotate90CW(up);
			}

			return up;
		}

		glm::vec3 cubeCoordToPos(const glm::ivec3& coord) const
		{
			const float gridHStep = cubeHSize / boardSize;
			const auto pos2d = glm::vec2(coord) * (gridHStep * 2) - cubeHSize + gridHStep;

			switch (coord.z)
			{
			case 0:
				return { pos2d, cubeHSize };
			case 1:
				return { -pos2d.x, pos2d.y, -cubeHSize };
			case 2:
				return { -cubeHSize, pos2d.y, pos2d.x };
			case 3:
				return { cubeHSize, pos2d.y, -pos2d.x };
			case 4:
				return { pos2d.x, -cubeHSize, pos2d.y };
			case 5:
				return { pos2d.x, cubeHSize, -pos2d.y };
			}

			return glm::vec3(0.0f);
		}

		const std::unordered_map<SnakeNode::Type, glm::vec4> snakeNodeColors = { { SnakeNode::Type::Head, snakeHeadColor }, { SnakeNode::Type::DeadHead, snakeDeadHeadColor },
			{ SnakeNode::Type::EatingHead, snakeEatingHeadColor }, { SnakeNode::Type::Tail, snakeTailColor }, { SnakeNode::Type::Food, snakeFoodColor } };
		const std::unordered_map<int, SnakeDirection> controls = { 
			{ 0x26/*VK_UP*/, SnakeDirection::Up },
			{ 0x28/*VK_DOWN*/, SnakeDirection::Down },
			{ 0x25/*VK_LEFT*/, SnakeDirection::Left },
			{ 0x27/*VK_RIGHT*/, SnakeDirection::Right } };
		std::array<CM::StaticTexture, 6> cubeTextures;
		std::array<std::unique_ptr<Tools::ColorBufferEditor<glm::vec4>>, 6> cubeEditors;
		float moveTime{};

		ComponentId marbleTexture{};

		SnakeNodes snakeNodes;
		SnakeNodes::iterator snakeHead;
		SnakeNodes::iterator snakeEnd;
		SnakeDirection snakeDirection{};
		glm::ivec2 lastSnakeStep{};
		glm::ivec2 up{};
		glm::ivec3 targetCameraUp{};
		int lenghteningLeft{};
		std::optional<glm::ivec3> foodPos;
		int score{};
		ComponentId crossesId{};
		ComponentId eatingSoundBuffer{};
		ComponentId growingSoundBuffer{};
		ComponentId deadSoundBuffer{};
		ComponentId growingSound{};
		ComponentId deadSound{};
		glm::vec3 lightColorOffset{};

		std::unordered_set<glm::ivec3> freeSpace;

		float transformBase = 0.0304f;
		std::future<void> transformFuture;
	};

	SnakeCube::SnakeCube():
		impl(std::make_unique<Impl>())
	{
		impl->setup();
	}

	void SnakeCube::step()
	{
		impl->step();
	}
}
