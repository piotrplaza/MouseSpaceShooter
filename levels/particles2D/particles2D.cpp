#include "particles2D.hpp"

#include <components/graphicsSettings.hpp>

#include <components/camera2D.hpp>
#include <components/decoration.hpp>
#include <components/particles.hpp>
#include <components/mouse.hpp>
#include <globals/components.hpp>

#include <tools/Shapes2D.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

namespace
{
	constexpr float mouseSensitivity = 0.002f;
	constexpr unsigned particlesCount = 1000000;
	constexpr glm::vec2 hSize = glm::vec2(0.5f);
	constexpr glm::vec2 initVelocityRange = glm::vec2(0.0f, 0.5f);
}

namespace Levels
{
	class Particles2D::Impl
	{
	public:
		void setup()
		{
			auto& graphicsSettings = Globals::Components().graphicsSettings();
			auto& camera = Globals::Components().camera2D();
			auto& particles = Globals::Components().particles();
			auto& decorations = Globals::Components().staticDecorations();

			//auto& cursor = decorations.emplace(Tools::Shapes2D::CreatePositionsOfCircle(glm::vec2(0.0f), 0.01f, 20));
			//cursor.modelMatrixF = [&]() { return glm::translate(glm::mat4(1.0f), glm::vec3(cursorPosition, 0.0f)); };

			graphicsSettings.pointSize = 1.0f;
			graphicsSettings.lineWidth = 1.0f;

			camera.targetPositionAndProjectionHSizeF = glm::vec3(0.0f, 0.0f, camera.details.projectionHSize = camera.details.prevProjectionHSize = 1.0f);

			createParticles();
		}

		void step()
		{
			if (!particlesId)
				return;

			const auto& camera = Globals::Components().camera2D();
			const auto& mouse = Globals::Components().mouse();
			auto& particles = Globals::Components().particles()[particlesId];

			cursorPosition += mouse.getCartesianDelta() * mouseSensitivity;
			cursorPosition = glm::clamp(cursorPosition, -camera.details.completeProjectionHSize, camera.details.completeProjectionHSize);
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
			const auto& mouse = Globals::Components().mouse();
			auto& camera = Globals::Components().camera2D();

			cameraProjectionHSize -= mouse.pressed.wheel * 0.5f;

			camera.targetPositionAndProjectionHSizeF = glm::vec3(0.0f, 0.0f, cameraProjectionHSize);
		}

		void createParticles()
		{
			const auto& camera = Globals::Components().camera2D();
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
				positions.emplace_back(glm::linearRand(-hSize, hSize), 0.0f);
				colors.emplace_back(glm::linearRand(glm::vec3(0.01f), glm::vec3(1.0f)), 1.0f);
				velocitiesAndTimes.emplace_back(glm::circularRand(glm::linearRand(initVelocityRange.x, initVelocityRange.y)), 0.0f, 0.0f);
				hSizesAndAngles.emplace_back(glm::vec3(0.0f));
			}
			
			if (particlesId)
				particles[particlesId].state = ComponentState::Outdated;

			auto& particles1 = particles.emplace(std::move(positions), std::move(colors), std::move(velocitiesAndTimes), std::move(hSizesAndAngles));
			particles1.tfRenderingSetupF = [&](ShadersUtils::ProgramBase& programBase) mutable {
				auto& program = static_cast<ShadersUtils::Programs::TFParticlesAccessor&>(programBase);
				program.deltaTime(0.0f);

				if (started)
					particles1.tfRenderingSetupF = nullptr;

				return nullptr;
			};

			particles1.bufferDataUsage = GL_DYNAMIC_COPY;
			particlesId = particles1.getComponentId();

			//billboards.emplace(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.2f, 0.0f), glm::vec2(-0.2f, 0.2f), 1000);
		}

		glm::vec2 cursorPosition{};
		ComponentId particlesId{};

		float cameraProjectionHSize = 1.0f;
		bool started = false;
	};

	Particles2D::Particles2D() :
		impl(std::make_unique<Impl>())
	{
	}

	void Particles2D::postSetup()
	{
		impl->setup();
	}

	Particles2D::~Particles2D() = default;

	void Particles2D::step()
	{
		impl->step();
	}
}
