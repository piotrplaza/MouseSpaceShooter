#include "Crosses3DTexturing.hpp"

#include <components/decoration.hpp>
#include <components/physics.hpp>
#include <components/graphicsSettings.hpp>
#include <components/light3D.hpp>
#include <components/camera3D.hpp>
#include <components/texture.hpp>

#include <globals/components.hpp>

#include <tools/shapes3D.hpp>
#include <tools/glmHelpers.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace Levels
{
	class Crosses3DTexturing::Impl
	{
	public:
		void setup()
		{
			glLineWidth(1.0f);

			Globals::Components().graphicsSettings().clearColor = { 0.0f, 0.05f, 0.0f, 1.0f };
			Globals::Components().camera3D().rotation = Components::Camera3D::LookAtRotation{};
			for(unsigned i = 0; i < 4; ++i)
				Globals::Components().lights3D().emplace(glm::vec3(0.0f), glm::vec3(1.0f), 0.1f, 0.0f);
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().textures();

			marbleTexture = textures.size();
			textures.emplace("textures/green marble.jpg").wrapMode = GL_MIRRORED_REPEAT;
		}

		void createDecorations() const
		{
			const glm::ivec2 numOfCrosses = { 200, 200 };
			const glm::vec2 distanceBetweenCrosses = { 1.5f, 1.0f };

			const auto& physics = Globals::Components().physics();
			auto& staticDecorations = Globals::Components().staticDecorations();

			Shapes3D::AddGrid(staticDecorations.emplace(), numOfCrosses * distanceBetweenCrosses, numOfCrosses);
			staticDecorations.last().params3D->ambient(1.0f);

			for (const auto& light: Globals::Components().lights3D())
			{
				Shapes3D::AddSphere(staticDecorations.emplace(), 0.2f, 2, 3);
				staticDecorations.last().colorF = [&]() { return glm::vec4(light.color, 1.0f) + Globals::Components().graphicsSettings().clearColor * light.clearColorFactor; };
				staticDecorations.last().params3D->lightModelEnabled(false);
				staticDecorations.last().modelMatrixF = [&]() { return glm::rotate(glm::translate(glm::mat4(1.0f), light.position), physics.simulationDuration * 4.0f, { 1.0f, 1.0f, 1.0f }); };
			}

			{
				const glm::vec3 offset = { (numOfCrosses.x - 1) * distanceBetweenCrosses.x / 2.0f, 0.0f, (numOfCrosses.y - 1) * distanceBetweenCrosses.y / 2.0f };

				staticDecorations.emplace();
				for (int z = 0; z < numOfCrosses.y; ++z)
					for (int x = 0; x < numOfCrosses.x; ++x)
						Shapes3D::AddCross(staticDecorations.last(), { 0.1f, 0.5f, 0.1f }, { 0.35f, 0.1f, 0.1f }, 0.15f, [](auto, glm::vec3 p) { return glm::vec2(p.x + p.z, p.y + p.z); }, glm::translate(glm::mat4(1.0f),
							glm::vec3(x * distanceBetweenCrosses.x, 0.0f, z * distanceBetweenCrosses.y) - offset));
				staticDecorations.last().params3D->ambient(0.4f).diffuse(0.8f).specular(0.8f).specularMaterialColorFactor(0.2f).lightModelEnabled(true);
				staticDecorations.last().texture = TCM::Texture(marbleTexture);
				//staticDecorations.last().instancing = Renderable::Instancing{}.addTransforms({ glm::mat4(1.0f), glm::scale(glm::mat4(1.0f), {1.0f, -1.0f, 1.0f}) });
			}
		}

		void cameraStep() const
		{
			const float height = 1.5f;
			const float rotationSpeed = 0.1f;
			const float oacilationSpeed = 0.5f;
			const float radius = 8.0f;
			const float oscilation = 0.8f;

			const auto& physics = Globals::Components().physics();
			auto& camera = Globals::Components().camera3D();

			camera.position = glm::vec3(glm::cos(physics.simulationDuration * rotationSpeed) * radius, height + glm::sin(physics.simulationDuration * oacilationSpeed) * oscilation,
				glm::sin(physics.simulationDuration * rotationSpeed) * radius);
		}

		void lightStep() const
		{
			const auto& physics = Globals::Components().physics();

			int i = 0;
			for (auto& light: Globals::Components().lights3D())
			{
				const float rotationSpeed = (-0.5f - (i * 0.2f)) * (i % 2 * 2.0f - 1.0f);
				const float radius = 5.0f + rotationSpeed;
				const glm::vec3 changeColorSpeed(1.0f, 0.6f, 0.3f);

				light.position = glm::vec3(glm::cos(physics.simulationDuration * rotationSpeed) * radius, 2.0f, glm::sin(physics.simulationDuration * rotationSpeed) * radius);
				light.color = { (glm::cos(physics.simulationDuration * changeColorSpeed.r * rotationSpeed) + 1.0f) / 2.0f,
					(glm::cos(physics.simulationDuration * changeColorSpeed.g * rotationSpeed) + 1.0f) / 2.0f,
					(glm::cos(physics.simulationDuration * changeColorSpeed.b * rotationSpeed) + 1.0f) / 2.0f };
				//light.color = glm::vec3(1.0f);
				++i;
			}
		}

	private:
		ComponentId marbleTexture = 0;
	};

	Crosses3DTexturing::Crosses3DTexturing() :
		impl(std::make_unique<Impl>())
	{
		impl->setup();
		impl->loadTextures();
		impl->createDecorations();
	}

	void Crosses3DTexturing::step()
	{
		impl->cameraStep();
		impl->lightStep();
	}
}
