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

namespace
{
	constexpr int boardSize = 9;
	constexpr float moveDuration = 0.5f;
	constexpr int lenghtening = 1;
	
	constexpr glm::vec4 clearColor = { 0.0f, 0.3f, 0.0f, 1.0f };
	constexpr glm::vec4 snakeHeadColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	constexpr glm::vec4 snakeTailColor = { 0.0f, 1.0f, 0.0f, 1.0f };
	constexpr glm::vec4 snakeFoodColor = { 0.0f, 0.0f, 1.0f, 1.0f };
	constexpr glm::vec4 cubeColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	
	constexpr float cubeHSize = 0.5f;
	constexpr float cameraDistance = 2.0f;
}

//#define TEST

namespace Levels
{
	class SnakeCube::Impl
	{
	public:
		void setup()
		{
			glDisable(GL_CULL_FACE);

			Globals::Components().graphicsSettings().clearColor = clearColor;
			Globals::Components().camera3D().rotation = Components::Camera3D::LookAtRotation{};

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

#ifdef TEST
			Shapes3D::CreateCuboid(Globals::Components().staticDecorations(), testCubeTextures, glm::vec3(cubeHSize) + 0.001f);
#endif
			
			Globals::Components().stepSetups().emplace([this]() {
				createEditors(); 
				gameplayInit();
				return false; 
			});

			{
				Shapes3D::AddSphere(Globals::Components().staticDecorations().emplace(), 0.02f, 20, 20, nullptr, false);
				Globals::Components().staticDecorations().last().colorF = []() { return glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); };
				Globals::Components().staticDecorations().last().modelMatrixF = [&]() { return glm::translate(glm::mat4(1.0f), cubeCoordToPos(snakeHead->first)); };
			}
		}

		void step()
		{
			controlsStep();

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
			enum class Type { Head, Tail, Food } type;
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

			snakeNodes = { { { cubeEditors[0]->getRes() / 2, 0 }, { SnakeNode::Type::Head, snakeNodes.end(), snakeNodes.end() } } };
			snakeHead = snakeEnd = snakeNodes.begin();
			snakeDirection = SnakeDirection::Up;
			lastSnakeStep = { 0, 0 };
			up = { 0, 1 };
			cameraUp = { 0, 1, 0 };
			lenghteningLeft = 0;

			drawSnake();
		}

		void gameplayStep()
		{
			const glm::ivec3 lastEndPos = snakeEnd->first;
			lastSnakeStep = snakeStep(snakeDirection);
			glm::ivec3 nextPos = snakeHead->first + glm::ivec3(lastSnakeStep, 0);
			handleBorders(nextPos);
			snakeHead = snakeNodes.emplace(nextPos, SnakeNode{ SnakeNode::Type::Head, snakeNodes.end(), snakeNodes.end() }).first;
			snakeNodes.erase(snakeEnd);
			snakeEnd = snakeHead;

			redrawSnake(lastEndPos);
		}

		void handleBorders(glm::ivec3& pos)
		{
			const glm::ivec2 prevUp = up;

			switch (pos[2])
			{
			case 0:
				if (pos.x < 0)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(cameraUp, 1));
					pos.x = boardSize - 1;
					pos.z = 2;
				}
				else if (pos.x >= boardSize)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(cameraUp, 1));
					pos.x = 0;
					pos.z = 3;
				}
				else if (pos.y < 0)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(cameraUp, 1));
					pos.y = boardSize - 1;
					pos.z = 4;
				}
				else if (pos.y >= boardSize)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(cameraUp, 1));
					pos.y = 0;
					pos.z = 5;
				}
				break;
			case 1:
				if (pos.x < 0)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(cameraUp, 1));
					pos.x = boardSize - 1;
					pos.z = 3;
				}
				else if (pos.x >= boardSize)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(cameraUp, 1));
					pos.x = 0;
					pos.z = 2;
				}
				else if (pos.y < 0)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(cameraUp, 1));
					up = Rotate180(up);
					pos = { boardSize - 1 - pos.x, 0, 4 };
				}
				else if (pos.y >= boardSize)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(cameraUp, 1));
					up = Rotate180(up);
					pos = { boardSize - 1 - pos.x, boardSize - 1, 5 };
				}
				break;
			case 2:
				if (pos.x < 0)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(cameraUp, 1));
					pos.x = boardSize - 1;
					pos.z = 1;
				}
				else if (pos.x >= boardSize)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(cameraUp, 1));
					pos.x = 0;
					pos.z = 0;
				}
				else if (pos.y < 0)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(cameraUp, 1));
					up = Rotate90CCW(up);
					pos = { 0, pos.x, 4 };
				}
				else if (pos.y >= boardSize)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(cameraUp, 1));
					up = Rotate90CW(up);
					pos = { 0, boardSize - 1 - pos.x, 5 };
				}
				break;
			case 3:
				if (pos.x < 0)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(cameraUp, 1));
					pos.x = boardSize - 1;
					pos.z = 0;
				}
				else if (pos.x >= boardSize)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 1, 0 }) * glm::ivec4(cameraUp, 1));
					pos.x = 0;
					pos.z = 1;
				}
				else if (pos.y < 0)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(cameraUp, 1));
					up = Rotate90CW(up);
					pos = { boardSize - 1, boardSize - 1 - pos.x, 4 };
				}
				else if (pos.y >= boardSize)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(cameraUp, 1));
					up = Rotate90CCW(up);
					pos = { boardSize - 1, pos.x, 5 };
				}
				break;
			case 4:
				if (pos.x < 0)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(cameraUp, 1));
					up = Rotate90CW(up);
					pos = { pos.y, 0, 2 };
				}
				else if (pos.x >= boardSize)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(cameraUp, 1));
					up = Rotate90CCW(up);
					pos = { boardSize - 1 - pos.y, 0, 3 };
				}
				else if (pos.y < 0)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(cameraUp, 1));
					up = Rotate180(up);
					pos = { boardSize - 1 - pos.x, 0, 1 };
				}
				else if (pos.y >= boardSize)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(cameraUp, 1));
					pos.y = 0;
					pos.z = 0;
				}
				break;
			case 5:
				if (pos.x < 0)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(cameraUp, 1));
					up = Rotate90CCW(up);
					pos = { boardSize - 1 - pos.y, boardSize - 1, 2 };
				}
				else if (pos.x >= boardSize)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 0, 0, 1 }) * glm::ivec4(cameraUp, 1));
					up = Rotate90CW(up);
					pos = { pos.y, boardSize - 1, 3 };	
				}
				else if (pos.y < 0)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(cameraUp, 1));
					pos.y = boardSize - 1;
					pos.z = 0;
				}
				else if (pos.y >= boardSize)
				{
					cameraUp = glm::round(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), { 1, 0, 0 }) * glm::ivec4(cameraUp, 1));
					up = Rotate180(up);
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

		void redrawSnake(const glm::ivec3& lastEndPos)
		{
			cubeEditors[lastEndPos[2]]->putColor(lastEndPos, cubeColor);
			cubeTextures[lastEndPos[2]].component->state = ComponentState::Changed;

			cubeEditors[snakeHead->first[2]]->putColor(snakeHead->first, snakeNodeColors.at(snakeHead->second.type));
			cubeTextures[snakeHead->first[2]].component->state = ComponentState::Changed;

			auto snakeNeck = snakeHead->second.next;
			if (snakeNeck != snakeNodes.end())
			{
				cubeEditors[snakeNeck->first[2]]->putColor(snakeNeck->first, snakeNodeColors.at(snakeNeck->second.type));
				cubeTextures[snakeNeck->first[2]].component->state = ComponentState::Changed;
			}
		}

		void controlsStep()
		{
			for (const auto& [key, direction]: controls)
				if (Globals::Components().keyboard().pressed[key])
				{
					if (lastSnakeStep == -snakeStep(direction))
						continue;
					snakeDirection = direction;
					break;
				}
		}

		void cameraStep() const
		{
			const auto& physics = Globals::Components().physics();
			auto& camera = Globals::Components().camera3D();

#if 0
			const float rotationSpeed = 2.0f;
			const float oscilationSpeed = 3.0f;
			const float radius = 2.0f;
			const float oscilation = 1.5f;

			camera.position = glm::vec3(glm::cos(physics.simulationDuration * rotationSpeed + glm::half_pi<float>()) * radius,
				glm::sin(physics.simulationDuration * oscilationSpeed) * oscilation,
				glm::sin(physics.simulationDuration * rotationSpeed + glm::half_pi<float>()) * radius);
#else
			camera.position = glm::normalize(cubeCoordToPos(snakeHead->first)) * cameraDistance;
			std::get<Components::Camera3D::LookAtRotation>(camera.rotation).up = cameraUp;
#endif
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

		const std::unordered_map<SnakeNode::Type, glm::vec4> snakeNodeColors = { { SnakeNode::Type::Head, snakeHeadColor },
			{ SnakeNode::Type::Tail, snakeTailColor }, { SnakeNode::Type::Food, snakeFoodColor } };
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
		glm::ivec3 cameraUp;
		int lenghteningLeft;
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
