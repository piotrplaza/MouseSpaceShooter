#include "basic3D.hpp"

#include <components/decoration.hpp>
#include <components/physics.hpp>
#include <components/graphicsSettings.hpp>
#include <components/light3D.hpp>
#include <components/camera3D.hpp>
#include <components/texture.hpp>

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
			glDisable(GL_CULL_FACE);

			Globals::Components().graphicsSettings().clearColor = { 0.0f, 0.05f, 0.0f, 1.0f };
			Globals::Components().camera3D().rotation = Components::Camera3D::LookAtRotation{};
			Globals::Components().lights3D().emplace(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f), 0.4f, 0.0f, true);
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().staticTextures();

			skullTexture = textures.size();
			textures.emplace("textures/skull rot.png");
		}

		void createDecorations() const
		{
			const auto& physics = Globals::Components().physics();
			auto& staticDecorations = Globals::Components().staticDecorations();

#define TEST 2
#if TEST == 0
			auto& shape = staticDecorations.emplace();
			Shapes3D::AddRectangle(shape, { 1.8f, 1.8f }, { { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } });
			//shape.params3D->specularMaterialColorFactor(1.0f);
#elif TEST == 1
			auto& shape = staticDecorations.emplace();
			Shapes3D::AddRectangle(shape, { 1.8f, 1.8f }, { { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } }, [](glm::vec2 d, auto) { return d; });
			shape.texture = TCM::StaticTexture(skullTexture);
			//shape.params3D->illuminationF([]() { return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); });
			//shape.params3D->specularMaterialColorFactor(1.0f);
			//shape.colorF = []() { return glm::vec4(1.0f); };
#elif TEST == 2
			auto& shape = staticDecorations.emplace();
			Shapes3D::AddCuboid(shape, { 0.8f, 0.8f, 0.8f }, { { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } }, [](glm::vec2 d, auto) { return d; });
			shape.modelMatrixF = [&]() { return glm::rotate(glm::mat4(1.0f), physics.simulationDuration, { 1.0f, 1.0f, 1.0f }); };
			shape.texture = TCM::StaticTexture(skullTexture);
			shape.params3D->illuminationF([]() { return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); });
			shape.params3D->specularMaterialColorFactor(1.0f).specular(8.0f);
			//shape.colorF = []() { return glm::vec4(1.0f); };
#elif TEST == 3
			auto& shape = staticDecorations.emplace();
			Shapes3D::AddCross(shape, { 0.2f, 1.0f, 0.2f }, { 0.7f, 0.2f, 0.2f }, 0.3f, [](auto, glm::vec3 p) { return glm::vec2(p.x, p.y - 1.2f); });
			shape.modelMatrixF = [&]() { return glm::rotate(glm::mat4(1.0f), physics.simulationDuration, { 1.0f, 1.0f, 1.0f }); };
			shape.texture = TCM::StaticTexture(skullTexture);
			shape.params3D->illuminationF([]() { return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); });
			//shape.params3D->specularMaterialColorFactor(1.0f);
			//shape.colorF = []() { return glm::vec4(1.0f); };
#elif TEST == 4
			auto& shape = staticDecorations.emplace();
			Shapes3D::AddCuboid(shape, { 0.5f, 0.5f, 0.5f }, { { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } });
			Shapes3D::AddSphere(shape, 0.65f, 50, 50, [](glm::vec3 normal) { return glm::vec4(normal, 1.0f); });
			for (const auto tX : {-0.52f, 0.52f})
				for (const auto tZ : { -0.52f, 0.52f })
					Shapes3D::AddSphere(shape, 0.5f, 50, 50, [](glm::vec3 normal) { return glm::vec4(normal, 1.0f); }, false,
						glm::scale(glm::translate(glm::mat4(1.0f), { tX, 0.0f, tZ }), {0.1f, 1.0f, 0.1f}));
			shape.modelMatrixF = [&]() { return glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, -2.0f }), physics.simulationDuration, { 1.0f, 1.0f, 1.0f }); };
#elif TEST == 5
			const int sphereComplexity = 64;
			const float size = 2.0f;
			auto scaleF = [](float t) { return std::min((std::cos(t * 0.5f) + 1.1f) / 2.0f, 1.0f); };
			auto localTransformF = [&]() { return glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, -2.0f }), physics.simulationDuration, { 1.0f, 1.0f, 1.0f }); };
			Shapes3D::AddSphere(staticDecorations.emplace(), 0.5f, sphereComplexity, sphereComplexity, [](glm::vec3 normal) { return glm::vec4(normal, 1.0f); });
			staticDecorations.last().modelMatrixF = [=, &physics]() { return glm::scale(localTransformF(), glm::vec3(1.0f, scaleF(physics.simulationDuration), scaleF(physics.simulationDuration)) * size); };
			Shapes3D::AddSphere(staticDecorations.emplace(), 0.5f, sphereComplexity, sphereComplexity, [](glm::vec3 normal) { return glm::vec4(normal, 1.0f); });
			staticDecorations.last().modelMatrixF = [=, &physics]() { return glm::scale(localTransformF(), glm::vec3(scaleF(physics.simulationDuration), 1.0f, scaleF(physics.simulationDuration)) * size); };
			Shapes3D::AddSphere(staticDecorations.emplace(), 0.5f, sphereComplexity, sphereComplexity, [](glm::vec3 normal) { return glm::vec4(normal, 1.0f); });
			staticDecorations.last().modelMatrixF = [=, &physics]() { return glm::scale(localTransformF(), glm::vec3(scaleF(physics.simulationDuration), scaleF(physics.simulationDuration), 1.0f) * size); };
#endif
		}

	private:
		ComponentId skullTexture = 0;
	};

	Basic3D::Basic3D():
		impl(std::make_unique<Impl>())
	{
		impl->setup();
		impl->loadTextures();
		impl->createDecorations();
	}

	void Basic3D::step()
	{
		const auto& physics = Globals::Components().physics();
		auto& camera = Globals::Components().camera3D();

		const float rotationSpeed = 0.8f;
		const float oacilationSpeed = 1.0f;
		const float radius = 4.0f;
		const float oscilation = 0.0f;

		camera.position = glm::vec3(glm::cos(physics.simulationDuration * rotationSpeed) * radius, glm::sin(physics.simulationDuration * oacilationSpeed) * oscilation,
			glm::sin(physics.simulationDuration * rotationSpeed) * radius);
	}
}
