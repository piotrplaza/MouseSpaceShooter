#include "Crosses3DInstancing.hpp"

#include <components/decoration.hpp>
#include <components/physics.hpp>
#include <components/graphicsSettings.hpp>
#include <components/light3D.hpp>
#include <components/camera3D.hpp>
#include <components/texture.hpp>
#include <components/keyboard.hpp>
#include <components/mouse.hpp>

#include <globals/components.hpp>

#include <tools/shapes3D.hpp>
#include <tools/glmHelpers.hpp>
#include <tools/utility.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <execution>
#include <future>

namespace Levels
{
	class Crosses3DInstancing::Impl
	{
	public:
		void setup()
		{
			Globals::Components().camera3D().farPlane = 1000000.0f;
			Globals::Components().graphicsSettings().clearColorF = glm::vec4{ 0.2f, 0.05f, 0.0f, 1.0f };
			Globals::Components().camera3D().rotation = Components::Camera3D::LookAtRotation{};
			for (unsigned i = 0; i < 40; ++i)
				Globals::Components().lights3D().emplace(glm::vec3(0.0f), glm::vec3(1.0f), 0.6f, 1.0f);
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().dynamicTextures();

			marbleTexture = textures.emplace("textures/green marble.jpg").getComponentId();
			textures.last().wrapMode = GL_MIRRORED_REPEAT;
		}

		void createDecorations() const
		{
			const int numOfCrosses = 10000;

			const auto& physics = Globals::Components().physics();
			auto& staticDecorations = Globals::Components().staticDecorations();
			auto& dynamicDecorations = Globals::Components().dynamicDecorations();

			for (const auto& light: Globals::Components().lights3D())
			{
				Shapes3D::AddSphere(staticDecorations.emplace(), 0.2f, 2, 3);
				staticDecorations.last().colorF = [&]() { return glm::vec4(light.color, 1.0f) + Globals::Components().graphicsSettings().clearColorF * light.clearColorFactor; };
				staticDecorations.last().params3D->lightModelEnabled(false);
				staticDecorations.last().modelMatrixF = [&]() { return glm::rotate(glm::translate(glm::mat4(1.0f), light.position), physics.simulationDuration * 4.0f, { 1.0f, 1.0f, 1.0f }); };
			}

			{
				Shapes3D::AddCross(dynamicDecorations.emplace(), { 0.1f, 0.5f, 0.1f }, { 0.35f, 0.1f, 0.1f }, 0.15f, [](auto, glm::vec3 p) { return glm::vec2(p.x + p.z, p.y + p.z); });
				dynamicDecorations.last().params3D->ambient(0.4f).diffuse(0.8f).specular(0.8f).specularMaterialColorFactor(0.2f).lightModelEnabled(true).gpuSideInstancedNormalTransforms(true);
				dynamicDecorations.last().texture = CM::DynamicTexture(marbleTexture);
				dynamicDecorations.last().bufferDataUsage = GL_DYNAMIC_DRAW;
				dynamicDecorations.last().instancing.emplace().init(numOfCrosses, glm::mat4(1.0f));
			}
		}

		void cameraStep()
		{
			const auto& physics = Globals::Components().physics();
			auto& camera = Globals::Components().camera3D();

			const float height = 1.5f;
			const float rotationSpeed = 0.5f;
			const float oscilationSpeed = 0.5f;
			const float radius = cameraDistanceBase + glm::sin(physics.simulationDuration * 0.2f) * 20.0f;
			const float oscilation = 0.8f;

			cameraDistanceBase -= Globals::Components().mouse().pressed.wheel * 10;

			camera.position = glm::vec3(glm::cos(physics.simulationDuration * rotationSpeed) * radius, height + glm::sin(physics.simulationDuration * oscilationSpeed) * oscilation,
				glm::sin(physics.simulationDuration * rotationSpeed) * radius);
		}

		void lightStep() const
		{
			const auto& physics = Globals::Components().physics();

			int i = 0;
			for (auto& light: Globals::Components().lights3D())
			{
				const float rotationSpeed = (-0.1f - (i * 0.03f)) * (i % 2 * 2.0f - 1.0f);
				const float radius = 1.0f + rotationSpeed * 5.0f;
				const glm::vec3 changeColorSpeed(1.0f, 0.6f, 0.3f);

				light.position = glm::vec3(glm::cos(physics.simulationDuration * rotationSpeed) * radius, glm::cos(physics.simulationDuration * rotationSpeed * 0.3f) * radius, glm::sin(physics.simulationDuration * rotationSpeed * 0.6f) * radius);
				light.color = { (glm::cos(physics.simulationDuration * changeColorSpeed.r * rotationSpeed) + 1.0f) / 2.0f,
					(glm::cos(physics.simulationDuration * changeColorSpeed.g * rotationSpeed) + 1.0f) / 2.0f,
					(glm::cos(physics.simulationDuration * changeColorSpeed.b * rotationSpeed) + 1.0f) / 2.0f };
				//light.color = glm::vec3(1.0f);
				++i;
			}
		}

		void transformStep()
		{
			auto& dynamicDecorations = Globals::Components().dynamicDecorations();

			if (dynamicDecorations.empty())
				return;

			const float transformSpeed = 0.00001f;
			const float transformBaseStep = 0.001f;

			const auto& physics = Globals::Components().physics();
			const auto& keyboard = Globals::Components().keyboard();
			auto& transforms = dynamicDecorations.last().instancing->transforms_;

			if (keyboard.pressed[0x26/*VK_UP*/])
				transformBase += transformBaseStep;
			if (keyboard.pressed[0x28/*VK_DOWN*/])
				transformBase -= transformBaseStep;
#if 1
			if (transformFuture.valid())
			{
				transformFuture.get();
				dynamicDecorations.last().state = ComponentState::Changed;
			}

			transformFuture = std::async(std::launch::async, [=, simulationDuration = physics.simulationDuration, &transforms]() {
				Tools::ItToId itToId(transforms.size());
				std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), [=, &transforms](const auto i) {
					transforms[i] = glm::rotate(glm::mat4(1.0f), i * glm::pi<float>() * 0.001f, { 1.0f, 0.0f, 0.0f })
						* glm::rotate(glm::mat4(1.0f), i * glm::pi<float>() * 0.03f, { 0.0f, 1.0f, 0.0f })
						* glm::rotate(glm::mat4(1.0f), i * glm::pi<float>() * (transformBase - simulationDuration * transformSpeed), { 0.0f, 0.0f, 1.0f })
						* glm::translate(glm::mat4(1.0f), { i * 0.0005f, i * 0.0007f, i * 0.0009f });
					});
			});
#else
			Tools::ItToId itToId(transforms.size());
			std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), [&](const auto i) {
					transforms[i] = glm::rotate(glm::mat4(1.0f), i * glm::pi<float>() * 0.001f, { 1.0f, 0.0f, 0.0f })
						* glm::rotate(glm::mat4(1.0f), i * glm::pi<float>() * 0.03f, { 0.0f, 1.0f, 0.0f })
						* glm::rotate(glm::mat4(1.0f), i * glm::pi<float>() * (transformBase - physics.simulationDuration * transformSpeed), { 0.0f, 0.0f, 1.0f })
						* glm::translate(glm::mat4(1.0f), { i * 0.0005f, i * 0.0007f, i * 0.0009f });
				});
			dynamicDecorations.last().state = ComponentState::Changed;
#endif

			if (keyboard.pressed['D'])
			{
				dynamicDecorations.last().state = ComponentState::Outdated;
				Globals::Components().dynamicTextures().last().state = ComponentState::Outdated;
				if (transformFuture.valid())
					transformFuture.get();
			}
		}

	private:
		ComponentId marbleTexture = 0;
		float transformBase = 0.005f;
		float cameraDistanceBase = 60.0f;
		std::future<void> transformFuture;
	};

	Crosses3DInstancing::Crosses3DInstancing() :
		impl(std::make_unique<Impl>())
	{
		impl->setup();
		impl->loadTextures();
		impl->createDecorations();
	}

	void Crosses3DInstancing::step()
	{
		impl->cameraStep();
		impl->lightStep();
		impl->transformStep();
	}
}
