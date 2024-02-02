#include "basic3D.hpp"

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
	class Basic3D::Impl
	{
	public:
		void setup() const
		{
			Globals::Components().graphicsSettings().clearColor = { 0.0f, 0.05f, 0.0f, 1.0f };
			Globals::Components().lights3D().emplace(glm::vec3(0.0f), glm::vec3(1.0f), 0.5f, true);
			Globals::Components().camera3D().rotation = Components::Camera3D::LookAtRotation{};
		}

		void createDecorations() const
		{
			auto& staticDecorations = Globals::Components().staticDecorations();
			const auto& physics = Globals::Components().physics();

			/*auto& shape = staticDecorations.emplace();
			Shapes3D::AddRectangle(shape, { 0.8f, 0.8f }, { { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } });
			shape.modelMatrixF = [&]() { return glm::rotate(glm::mat4(1.0f), physics.simulationDuration, { 0.0f, 1.0f, 0.0f }); };*/
#if 0
			auto& shape = staticDecorations.emplace();
			Shapes3D::AddCuboid(shape, { 0.5f, 0.5f, 0.5f }, { { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } });
			Shapes3D::AddSphere(shape, 0.65f, 50, 50, false, [](glm::vec3 normal) { return glm::vec4(normal, 1.0f); });
			for (const auto tX : {-0.52f, 0.52f})
				for (const auto tZ : { -0.52f, 0.52f })
					Shapes3D::AddSphere(shape, 0.5f, 50, 50, false, [](glm::vec3 normal) { return glm::vec4(normal, 1.0f); },
						glm::scale(glm::translate(glm::mat4(1.0f), { tX, 0.0f, tZ }), {0.1f, 1.0f, 0.1f}));
			shape.modelMatrixF = [&]() { return glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, -2.0f }), physics.simulationDuration, { 1.0f, 1.0f, 1.0f }); };
#else
			const int sphereComplexity = 64;
			const float size = 2.0f;
			auto scaleF = [](float t) { return std::min((std::cos(t * 0.5f) + 1.1f) / 2.0f, 1.0f); };
			Shapes3D::AddSphere(staticDecorations.emplace(), 0.5f, sphereComplexity, sphereComplexity, false, [](glm::vec3 normal) { return glm::vec4(normal, 1.0f); });
			staticDecorations.last().modelMatrixF = [&, size]() { return glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, scaleF(physics.simulationDuration), scaleF(physics.simulationDuration)) * size); };
			Shapes3D::AddSphere(staticDecorations.emplace(), 0.5f, sphereComplexity, sphereComplexity, false, [](glm::vec3 normal) { return glm::vec4(normal, 1.0f); });
			staticDecorations.last().modelMatrixF = [&, size]() { return glm::scale(glm::mat4(1.0f), glm::vec3(scaleF(physics.simulationDuration), 1.0f, scaleF(physics.simulationDuration)) * size); };
			Shapes3D::AddSphere(staticDecorations.emplace(), 0.5f, sphereComplexity, sphereComplexity, false, [](glm::vec3 normal) { return glm::vec4(normal, 1.0f); });
			staticDecorations.last().modelMatrixF = [&, size]() { return glm::scale(glm::mat4(1.0f), glm::vec3(scaleF(physics.simulationDuration), scaleF(physics.simulationDuration), 1.0f) * size); };
#endif
		}
	};

	Basic3D::Basic3D():
		impl(std::make_unique<Impl>())
	{
		impl->setup();
		impl->createDecorations();
	}

	void Basic3D::step()
	{
		const auto& physics = Globals::Components().physics();
		auto& camera = Globals::Components().camera3D();

		const float rotationSpeed = 0.8f;
		const float oacilationSpeed = 1.0f;
		const float radius = 4.0f;
		const float oscilation = 0.5f;

		camera.position = glm::vec3(glm::cos(physics.simulationDuration * rotationSpeed) * radius, glm::sin(physics.simulationDuration * oacilationSpeed) * oscilation,
			glm::sin(physics.simulationDuration * rotationSpeed) * radius);
	}
}
