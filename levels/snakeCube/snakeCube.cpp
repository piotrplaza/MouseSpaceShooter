#include "snakeCube.hpp"

#include <components/decoration.hpp>
#include <components/physics.hpp>
#include <components/graphicsSettings.hpp>
#include <components/camera3D.hpp>
#include <components/texture.hpp>
#include <components/functor.hpp>
#include <components/keyboard.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <tools/shapes3D.hpp>
#include <tools/colorBufferEditor.hpp>
#include <tools/glmHelpers.hpp>

#include <glm/gtx/hash.hpp>

#include <array>
#include <list>
#include <unordered_set>

#include <iostream>

//#define TEST

namespace
{
	constexpr int boardSize = 21;
	constexpr float moveDuration = 0.1f;
	constexpr int lenghtening = 80;
	
	constexpr glm::vec4 clearColor = { 0.0f, 0.4f, 0.6f, 1.0f };
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
}

namespace Levels
{
	class SnakeCube::Impl
	{
	public:
		void setup()
		{
			auto& graphicsSettings = Globals::Components().graphicsSettings();

			graphicsSettings.clearColor = clearColor;
			graphicsSettings.cullFace = false;
			graphicsSettings.lineWidth = 1.0f;

#ifdef TEST
			std::array<CM::StaticTexture, 6> testCubeTextures;
#endif

			for (size_t i = 0; i < 6; ++i)
			{
				auto& texture = cubeTextures[i];
#ifdef TEST
				testCubeTextures[i] = &Globals::Components().staticTextures().emplace(TextureData(TextureFile("textures/test/" + std::to_string(i) + ".png", 4)));
#endif
				texture = &Globals::Components().dynamicTextures().emplace(TextureData(std::vector<glm::vec4>(boardSize * boardSize, glm::vec4(0.0f)), glm::ivec2(boardSize)));
				texture.component->magFilter = GL_NEAREST;
				texture.component->wrapMode = GL_CLAMP_TO_EDGE;
			}

			Shapes3D::CreateCuboid(Globals::Components().staticDecorations(), cubeTextures, glm::vec3(cubeHSize));
			Shapes3D::AddWiredCuboid(Globals::Components().staticDecorations().emplace(), glm::vec3(cubeHSize - 0.001f), wiredCubeColor);

#ifdef TEST
			auto cuboidWalls = Shapes3D::CreateCuboid(Globals::Components().staticDecorations(), testCubeTextures, glm::vec3(cubeHSize) + 0.001f);
			for (auto* wall : cuboidWalls)
				wall->colorF = []() { return glm::vec4(0.2f); };
#endif
			Shapes3D::AddSphere(Globals::Components().staticDecorations().emplace(), 0.2f / boardSize, 20, 20, nullptr, false);
			Globals::Components().staticDecorations().last().colorF = []() { return snakeHeadSphereColor; };
			Globals::Components().staticDecorations().last().modelMatrixF = [&]() { return glm::translate(glm::mat4(1.0f), cubeCoordToPos(snakeHead->first)); };

			Globals::Components().stepSetups().emplace([this]() {
				createEditors(); 
				gameplayInit();
				return false; 
			});
		}

		void step()
		{
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
			enum class Type { Head, DeadHead, EatingHead, Tail, Food } type;
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

			freeSpace.clear();
			for (int z = 0; z < 6; ++z)
				for (int y = 0; y < boardSize; ++y)
					for (int x = 0; x < boardSize; ++x)
						freeSpace.insert({ x, y, z });
			freeSpace.erase(snakeHead->first);

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

		void redrawSnake(std::optional<glm::ivec3> erasedEndPos)
		{
			if (erasedEndPos)
			{
				const auto pos = *erasedEndPos;
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
		float moveTime = 0.0f;

		SnakeNodes snakeNodes;
		SnakeNodes::iterator snakeHead;
		SnakeNodes::iterator snakeEnd;
		SnakeDirection snakeDirection;
		glm::ivec2 lastSnakeStep;
		glm::ivec2 up;
		glm::ivec3 targetCameraUp;
		int lenghteningLeft;
		std::optional<glm::ivec3> foodPos;
		int score;

		std::unordered_set<glm::ivec3> freeSpace;
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
