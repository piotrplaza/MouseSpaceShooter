#include "Crosses3D.hpp"

#include <components/decoration.hpp>
#include <components/physics.hpp>
#include <components/graphicsSettings.hpp>
#include <components/light3D.hpp>
#include <components/camera3D.hpp>

#include <ogl/shaders/basicPhong.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <tools/shapes3D.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace Levels
{
	class Crosses3D::Impl
	{
	public:
		void setup() const
		{
			Globals::Components().graphicsSettings().clearColor = { 0.0f, 0.05f, 0.0f, 1.0f };
			Globals::Components().lights3D().emplace(glm::vec3(0.0f, 5.0f, 10.0f), glm::vec3(1.0f), 4.0f, false);
			Globals::Components().camera3D().rotation = Components::Camera3D::LookAtRotation{};
		}

		void createDecorations() const
		{
			auto& staticDecorations = Globals::Components().staticDecorations();

			{
				staticDecorations.emplace();
				Shapes3D::AddGrid(staticDecorations.last(), { 300.0f, 200.0f }, { 200, 200 });
			}

			{
				const glm::ivec2 numOfCrosses = { 200, 200 };
				const glm::vec2 distanceBetweenCrosses = { 1.5f, 1.0f };
				const glm::vec3 offset = { (numOfCrosses.x - 1) * distanceBetweenCrosses.x / 2.0f, 0.0f, (numOfCrosses.y - 1) * distanceBetweenCrosses.y / 2.0f };

				staticDecorations.emplace();
				for (int z = 0; z < numOfCrosses.y; ++z)
					for (int x = 0; x < numOfCrosses.x; ++x)
						Shapes3D::AddCross(staticDecorations.last(), { 0.1f, 0.5f, 0.1f }, { 0.35f, 0.1f, 0.1f }, 0.15f, glm::translate(glm::mat4(1.0f),
							glm::vec3(x * distanceBetweenCrosses.x, 0.0f, z * distanceBetweenCrosses.y) - offset));
				//staticDecorations.last().params3D->ambient(1.0f).diffuse(0.0f).specular(0.0f);
			}
		}

		void camera() const
		{
			const auto& physics = Globals::Components().physics();
			auto& camera = Globals::Components().camera3D();

			const float height = 1.5f;
			const float rotationSpeed = 0.8f;
			const float oacilationSpeed = 1.0f;
			const float radius = 4.0f;
			const float oscilation = 0.4f;

			camera.position = glm::vec3(glm::cos(physics.simulationDuration * rotationSpeed) * radius, height + glm::sin(physics.simulationDuration * oacilationSpeed) * oscilation,
				glm::sin(physics.simulationDuration * rotationSpeed) * radius);
		}
	};

	Crosses3D::Crosses3D():
		impl(std::make_unique<Impl>())
	{
		impl->setup();
		impl->createDecorations();
	}

	void Crosses3D::step()
	{
		impl->camera();
	}
}
