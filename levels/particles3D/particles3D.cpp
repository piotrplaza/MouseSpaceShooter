#include "particles3D.hpp"

#include <components/graphicsSettings.hpp>
#include <components/camera3D.hpp>
#include <components/decoration.hpp>
#include <components/particles.hpp>
#include <components/mouse.hpp>
#include <components/physics.hpp>
#include <globals/components.hpp>

#include <tools/Shapes3D.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

namespace
{
	constexpr float mouseSensitivity = 0.01f;
	constexpr unsigned particlesCount = 1000000;
	constexpr glm::vec3 hSize = glm::vec3(0.5f);
	constexpr glm::vec2 initVelocityRange = glm::vec2(0.0f, 0.5f);
}

namespace Levels
{
	class Particles3D::Impl
	{
	public:
		void setup()
		{
			auto& graphicsSettings = Globals::Components().graphicsSettings();
			auto& camera = Globals::Components().camera3D();
			auto& decorations = Globals::Components().decorations();

			//auto& cursor = decorations.emplace();
			//Tools::Shapes3D::AddSphere(cursor, 0.01f, 50, 50, nullptr, false, glm::mat4(1.0f));
			//cursor.modelMatrixF = [&]() { return glm::translate(glm::mat4(1.0f), glm::vec3(cursorPosition, 0.0f)); };

			graphicsSettings.force3D = true;
			graphicsSettings.pointSize = 1.0f;

			camera.rotation = Components::Camera3D::LookAtRotation{};

			createParticles();
		}

		void step()
		{
			const auto& camera = Globals::Components().camera3D();
			const auto& mouse = Globals::Components().mouse();
			auto& particles = Globals::Components().particles()[particlesId];

			cursorPosition += mouse.getCartesianDelta() * mouseSensitivity;
			cursorPosition = glm::clamp(cursorPosition, -glm::vec2(hSize), glm::vec2(hSize));
			particles.centers.emplace_back(cursorPosition, 0.0f);

			if (mouse.pressed.rmb)
			{
				createParticles();
				started = false;
			}

			if (mouse.pressed.lmb)
				started = true;

			cameraStep();
		}

	private:
		void cameraStep()
		{
			const float height = 1.5f;
			const float rotationSpeed = 0.5f;
			const float oscilationSpeed = 0.5f;
			const float oscilation = 0.8f;

			const auto& physics = Globals::Components().physics();
			const auto& mouse = Globals::Components().mouse();
			auto& camera = Globals::Components().camera3D();

			cameraRadius -= mouse.pressed.wheel * 0.5f;

			camera.position = glm::vec3(glm::cos(physics.simulationDuration * rotationSpeed) * cameraRadius, height + glm::sin(physics.simulationDuration * oscilationSpeed) * oscilation,
				glm::sin(physics.simulationDuration * rotationSpeed) * cameraRadius);
		}

		void createParticles()
		{
			auto& particles = Globals::Components().particles();

			std::vector<glm::vec3> positions;
			std::vector<glm::vec4> colors;
			std::vector<glm::vec4> velocitiesAndTimes;
			std::vector<glm::vec3> hSizesAndAngles;

			positions.reserve(particlesCount);
			colors.reserve(particlesCount);
			velocitiesAndTimes.reserve(particlesCount);
			hSizesAndAngles.reserve(particlesCount);
			for (unsigned i = 0; i < particlesCount; ++i)
			{
				positions.emplace_back(glm::linearRand(-hSize, hSize));
				colors.emplace_back(glm::linearRand(glm::vec3(0.01f), glm::vec3(1.0f)), 1.0f);
				velocitiesAndTimes.emplace_back(glm::sphericalRand(glm::linearRand(initVelocityRange.x, initVelocityRange.y)), 0.0f);
				hSizesAndAngles.emplace_back(glm::vec3(0.0f));
			}

			if (particlesId)
				particles[particlesId].state = ComponentState::Outdated;

			auto& particles1 = particles.emplace(std::move(positions), std::move(colors), std::move(velocitiesAndTimes), std::move(hSizesAndAngles));
			particles1.tfRenderingSetupF = [&, deltaTime = UniformsUtils::Uniform1f()](const auto& program) mutable {
				if (!deltaTime.isValid())
					deltaTime = UniformsUtils::Uniform1f(program, "deltaTime");

				deltaTime(0.0f);

				if (started)
					particles1.tfRenderingSetupF = nullptr;

				return nullptr;
			};

			particlesId = particles1.getComponentId();
		}

		glm::vec2 cursorPosition{};
		ComponentId particlesId{};

		float cameraRadius = 2.0f;
		bool started = false;
	};

	Particles3D::Particles3D():
		impl(std::make_unique<Impl>())
	{
	}

	void Particles3D::postSetup()
	{
		impl->setup();
	}

	Particles3D::~Particles3D() = default;

	void Particles3D::step()
	{
		impl->step();
	}
}
