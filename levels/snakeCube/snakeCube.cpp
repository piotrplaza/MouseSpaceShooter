#include "snakeCube.hpp"

#include <components/decoration.hpp>
#include <components/physics.hpp>
#include <components/graphicsSettings.hpp>
#include <components/camera3D.hpp>
#include <components/texture.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <tools/shapes3D.hpp>
#include <tools/colorBufferEditor.hpp>

#include <array>

namespace
{
	int boardSize = 5;
	float moveDuration = 0.5f;
}

namespace Levels
{
	class SnakeCube::Impl
	{
	public:
		void setup() const
		{
			glDisable(GL_CULL_FACE);

			Globals::Components().graphicsSettings().clearColor = { 0.0f, 0.05f, 0.0f, 1.0f };
			Globals::Components().camera3D().rotation = Components::Camera3D::LookAtRotation{};
		}

		void loadTextures()
		{
			for (size_t i = 0; i < 6; ++i)
			{
				auto& texture = cubeTextures[i];
				//texture = &Globals::Components().staticTextures().emplace(TextureData(TextureFile("textures/pp.png")));
				texture = &Globals::Components().dynamicTextures().emplace(TextureData(std::vector<glm::vec4>(boardSize * boardSize, glm::vec4(glm::vec3(1.0f), 1.0f)), glm::ivec2(boardSize)));
				texture.component->magFilter = GL_NEAREST;
				//texture.component->wrapMode = GL_CLAMP_TO_EDGE;

				auto& loadedTextureData = std::get<TextureData>(texture.component->source).loaded;
				auto& buffer = std::get<std::vector<glm::vec4>>(loadedTextureData.data);
				const auto& textureSize = loadedTextureData.size;
				cubeEditors[i] = std::make_unique<Tools::ColorBufferEditor<glm::vec4>>(buffer, textureSize);
			}
		}

		void createCube() const
		{
			Shapes3D::CreateCuboid(Globals::Components().staticDecorations(), cubeTextures);
		}

		void cameraStep() const
		{
			const auto& physics = Globals::Components().physics();
			auto& camera = Globals::Components().camera3D();

			const float rotationSpeed = 2.0f;
			const float oscilationSpeed = 2.0f;
			const float radius = 4.0f;
			const float oscilation = 1.5f;

			camera.position = glm::vec3(glm::cos(physics.simulationDuration * rotationSpeed) * radius, glm::sin(physics.simulationDuration * oscilationSpeed) * oscilation,
				glm::sin(physics.simulationDuration * rotationSpeed) * radius);
		}

		void gameplayStep()
		{
			const auto& physics = Globals::Components().physics();
			moveTime += physics.frameDuration;

			if (moveTime < moveDuration)
				return;

			moveTime -= moveDuration;

			for (size_t i = 0; i < 6; ++i)
			{
				auto& editor = *cubeEditors[i];
				auto& texture = *cubeTextures[i].component;

				editor.putColor({ Tools::RandomInt(0, editor.getRes().x - 1), Tools::RandomInt(0, editor.getRes().y - 1) },
					glm::vec4(Tools::RandomFloat(0.0f, 1.0f), Tools::RandomFloat(0.0f, 1.0f), Tools::RandomFloat(0.0f, 1.0f), Tools::RandomInt(0, 1)));

				texture.state = ComponentState::Changed;
			}
		}

	private:
		std::array<CM::StaticTexture, 6> cubeTextures;
		std::array<std::unique_ptr<Tools::ColorBufferEditor<glm::vec4>>, 6> cubeEditors;
		float moveTime = 0.0f;
	};

	SnakeCube::SnakeCube():
		impl(std::make_unique<Impl>())
	{
		impl->setup();
		impl->loadTextures();
		impl->createCube();
	}

	void SnakeCube::step()
	{
		impl->cameraStep();
		impl->gameplayStep();
	}
}
