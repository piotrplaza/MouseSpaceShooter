#include "particles.hpp"

#include <components/graphicsSettings.hpp>

#include <components/camera2D.hpp>
#include <components/decoration.hpp>
#include <components/particles.hpp>
#include <components/mouse.hpp>
#include <globals/components.hpp>

#include <tools/Shapes2D.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

#include <iostream>
using namespace std;

namespace Levels
{
	class Particles::Impl
	{
	public:
		void setup()
		{
			auto& camera = Globals::Components().camera2D();
			auto& cursor = Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreateVerticesOfCircle(glm::vec2(0.0f), 1.0f, 20));
			auto& particles = Globals::Components().staticParticles();

			camera.targetProjectionHSizeF = 100.0f;
			cursor.modelMatrixF = [&]() { return glm::translate(glm::mat4(1.0f), glm::vec3(cursorPosition, 0.0f)); };

			const auto particlesCount = 1000;
			std::vector<glm::vec3> positions;
			positions.reserve(particlesCount);
			for (unsigned i = 0; i < particlesCount; ++i)
				positions.emplace_back(glm::linearRand(glm::vec2(camera.details.projectionHSize), glm::vec2(camera.details.projectionHSize)), 0.0f);
			std::vector<glm::vec4> colors;
			colors.reserve(particlesCount);
			for (unsigned i = 0; i < particlesCount; ++i)
				colors.emplace_back(0.0f, 1.0f, glm::length(positions[i]) / camera.details.projectionHSize, 1.0f);
			particlesId = particles.emplace(std::move(positions), std::move(colors)).getComponentId();
		}

		void step()
		{
			auto& camera = Globals::Components().camera2D();
			auto& mouse = Globals::Components().mouse();
			auto& particles = Globals::Components().staticParticles()[particlesId];
			cursorPosition += mouse.getCartesianDelta() * 0.1f;
			cursorPosition = glm::clamp(cursorPosition, -camera.details.completeProjectionHSize, camera.details.completeProjectionHSize);
			particles.center = glm::vec3(cursorPosition, 0.0f);
		}

	private:
		glm::vec2 cursorPosition;
		ComponentId particlesId;
	};

	Particles::Particles():
		impl(std::make_unique<Impl>())
	{
		impl->setup();
	}

	Particles::~Particles() = default;

	void Particles::step()
	{
		impl->step();
	}
}
