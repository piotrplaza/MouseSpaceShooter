#include "snakeCube.hpp"

#include <components/decoration.hpp>
#include <components/physics.hpp>
#include <components/graphicsSettings.hpp>
#include <components/camera3D.hpp>
#include <components/texture.hpp>
#include <components/functor.hpp>

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

	constexpr glm::vec4 snakeHeadColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	constexpr glm::vec4 snakeTailColor = { 0.0f, 1.0f, 0.0f, 1.0f };
	constexpr glm::vec4 snakeFoodColor = { 0.0f, 0.0f, 1.0f, 1.0f };
	constexpr glm::vec4 cubeColor = { 0.0f, 0.0f, 0.0f, 1.0f };
}

namespace Levels
{
	class SnakeCube::Impl
	{
	public:
		void setup()
		{
			glDisable(GL_CULL_FACE);

			Globals::Components().graphicsSettings().clearColor = { 0.0f, 0.05f, 0.0f, 1.0f };
			Globals::Components().camera3D().rotation = Components::Camera3D::LookAtRotation{};

			for (size_t i = 0; i < 6; ++i)
			{
				auto& texture = cubeTextures[i];
				//texture = &Globals::Components().staticTextures().emplace(TextureData(TextureFile("textures/test/" + std::to_string(i) + ".png", 4)));
				texture = &Globals::Components().dynamicTextures().emplace(TextureData(std::vector<glm::vec4>(boardSize * boardSize, glm::vec4(0.0f)), glm::ivec2(boardSize)));
				texture.component->magFilter = GL_NEAREST;
				texture.component->wrapMode = GL_CLAMP_TO_EDGE;
			}

			Shapes3D::CreateCuboid(Globals::Components().staticDecorations(), cubeTextures);
			
			Globals::Components().stepSetups().emplace([this]() {
				createEditors(); 
				gameplayInit();
				return false; 
			});
		}

		void cameraStep() const
		{
			const auto& physics = Globals::Components().physics();
			auto& camera = Globals::Components().camera3D();

			const float rotationSpeed = 0.0f;
			const float oscilationSpeed = 0.0f;
			const float radius = 4.0f;
			const float oscilation = 1.5f;

			camera.position = glm::vec3(glm::cos(physics.simulationDuration * rotationSpeed - glm::half_pi<float>()) * radius, glm::sin(physics.simulationDuration * oscilationSpeed) * oscilation,
				glm::sin(physics.simulationDuration * rotationSpeed - glm::half_pi<float>()) * radius);
		}

		void step()
		{
			const auto& physics = Globals::Components().physics();
			moveTime += physics.frameDuration;

			if (moveTime < moveDuration)
				return;

			moveTime -= moveDuration;

			gameplayStep();
		}

		void gameplayInit()
		{
			for (size_t i = 0; i < 6; ++i)
			{
				cubeEditors[i]->clear(cubeColor);
				cubeTextures[i].component->state = ComponentState::Changed;
			}

			snakeNodes = { { { cubeEditors[0]->getRes() / 2, (int)CubeFace::Front }, SnakeNode::Head } };
			snakeHead = snakeEnd = snakeNodes.begin();
			snakeDirection = SnakeDirection::Up;
			up = { 0, 1 };
			lenghteningLeft = 0;

			drawSnake();
		}

	private:
		void createEditors()
		{
			for (size_t i = 0; i < 6; ++i)
			{
				auto& texture = cubeTextures[i];
				auto& loadedTextureData = std::get<TextureData>(texture.component->source).loaded;
				auto& buffer = std::get<std::vector<glm::vec4>>(loadedTextureData.data);
				const auto& textureSize = loadedTextureData.size;
				cubeEditors[i] = std::make_unique<Tools::ColorBufferEditor<glm::vec4>>(buffer, textureSize);
			}
		}

		void gameplayStep()
		{
			for (auto& [pos, node] : snakeNodes)
			{
				auto& editor = *cubeEditors[pos[2]];
				auto& texture = *cubeTextures[pos[2]].component;

				editor.putColor(pos, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			}

			const glm::ivec3 prevEndPos = snakeEnd->first;
			const auto newHead = snakeNodes.emplace(snakeHead->first + glm::ivec3(nextMove(), 0), SnakeNode::Head).first;
			snakeNodes.erase(snakeHead);
			snakeHead = snakeEnd = newHead;

			redrawSnake(prevEndPos);
		}

		void drawSnake()
		{
			for (auto& [pos, node] : snakeNodes)
			{
				cubeEditors[pos[2]]->putColor(pos, snakeNodeColors.at(node));
				cubeTextures[pos[2]].component->state = ComponentState::Changed;
			}
		}

		void redrawSnake(const glm::ivec3& prevEndPos)
		{
			cubeEditors[prevEndPos[2]]->putColor(prevEndPos, cubeColor);
			cubeTextures[prevEndPos[2]].component->state = ComponentState::Changed;

			cubeEditors[prevEndPos[2]]->putColor(snakeHead->first, snakeHeadColor);
			cubeTextures[prevEndPos[2]].component->state = ComponentState::Changed;
		}

		glm::ivec2 nextMove() const
		{
			switch (snakeDirection)
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

		enum class CubeFace { Back, Front, Left, Right, Top, Bottom };
		enum class SnakeNode { Head, Tail, Food };
		enum class SnakeDirection { Up, Down, Left, Right };

		const std::unordered_map<SnakeNode, glm::vec4> snakeNodeColors = { { SnakeNode::Head, snakeHeadColor }, { SnakeNode::Tail, snakeTailColor }, { SnakeNode::Food, snakeFoodColor } };
		std::array<CM::StaticTexture, 6> cubeTextures;
		std::array<std::unique_ptr<Tools::ColorBufferEditor<glm::vec4>>, 6> cubeEditors;
		float moveTime = 0.0f;

		std::unordered_map<glm::ivec3, SnakeNode> snakeNodes;
		decltype(snakeNodes)::iterator snakeHead;
		decltype(snakeNodes)::iterator snakeEnd;
		SnakeDirection snakeDirection;
		glm::ivec2 up;
		int lenghteningLeft;
	};

	SnakeCube::SnakeCube():
		impl(std::make_unique<Impl>())
	{
		impl->setup();
	}

	void SnakeCube::step()
	{
		impl->cameraStep();
		impl->step();
	}
}
