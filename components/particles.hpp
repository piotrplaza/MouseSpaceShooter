#pragma once

#include "_componentBase.hpp"
#include "_renderable.hpp"

#include <components/physics.hpp>
#include <globals/components.hpp>

#include <globals/shaders.hpp>
#include <ogl/shaders/tfParticles.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>

namespace Components
{
	struct Particles : Renderable
	{
		Particles() = default;

		Particles(std::vector<glm::vec3> positions, std::vector<glm::vec4> colors, std::vector<glm::vec4> velocitiesAndTimes,
			std::vector<glm::vec3> hSizesAndAngles, FVec2 lifeTimeRangeF, std::function<bool()> startedF, std::function<void()> outdatedF) :
			Renderable(std::move(positions), std::move(colors)),
			velocitiesAndTimes(std::move(velocitiesAndTimes)),
			hSizesAndAngles(std::move(hSizesAndAngles))
		{
			tfShaderProgram = &Globals::Shaders().tfParticles();

			tfRenderingSetupF = [=](auto& programBase) {
				auto& tfParticles = static_cast<ShadersUtils::Programs::TFParticles&>(programBase);
				tfParticles.init(true);
				tfParticles.respawning(false);
				tfParticles.lifeTimeRange(lifeTimeRangeF());

				return [=, &tfParticles]() {
					tfParticles.init(false);
					tfRenderingSetupF = nullptr;
				};
			};

			const auto& physics = Globals::Components().physics();
			stepF = [&, startTime = -1.0f, duration = lifeTimeRangeF().y, startedF = std::move(startedF), outdatedF = std::move(outdatedF)]() mutable {
				if (startTime == -1.0f)
					if (startedF())
						startTime = physics.simulationDuration;
					else
						return;

				if (physics.simulationDuration - startTime > duration)
				{
					state = ComponentState::Outdated;
					outdatedF();
				}
			};

			drawMode = GL_POINTS;
			bufferDataUsage = GL_DYNAMIC_COPY;
		}

		Particles(FVec3 sourcePointF, FVec3 initVelocityF, FVec2 lifeTimeRangeF, std::array<FVec4, 2> colorRangeF, glm::vec2 velocitySpreadRange,
			float velocityRotateZHRange, glm::vec3 gravity, bool respawning, unsigned particlesCount)
		{
			positions.push_back(sourcePointF());
			forcedPositionsCount = particlesCount;
			colors.push_back(glm::vec4(1.0f));
			forcedColorsCount = particlesCount;
			velocitiesAndTimes.push_back(glm::vec4(initVelocityF(), 0.0f));
			forcedVelocitiesAndTimesCount = particlesCount;
			hSizesAndAngles.push_back(glm::vec3(0.0f));
			forcedHSizesAndAnglesCount = particlesCount;

			tfShaderProgram = &Globals::Shaders().tfParticles();
			tfRenderingSetupF = [=](auto& programBase) {
				auto& tfParticles = static_cast<ShadersUtils::Programs::TFParticles&>(programBase);
				tfParticles.init(true);
				tfParticles.origin(sourcePointF());
				tfParticles.initVelocity(initVelocityF());
				tfParticles.lifeTimeRange(lifeTimeRangeF());
				tfParticles.colorRange({colorRangeF[0](), colorRangeF[1]()});
				tfParticles.velocitySpreadFactorRange(velocitySpreadRange);
				tfParticles.velocityRotateZHRange(velocityRotateZHRange);
				tfParticles.gravity(gravity);
				tfParticles.respawning(respawning);

				return [=, &tfParticles]() {
					tfParticles.init(false);
					tfRenderingSetupF = [=, &tfParticles](auto) {
						tfParticles.origin(sourcePointF());
						tfParticles.initVelocity(initVelocityF());
						tfParticles.lifeTimeRange(lifeTimeRangeF());
						tfParticles.colorRange({colorRangeF[0](), colorRangeF[1]()});
						return nullptr;
					};
				};
			};

			if (!respawning)
			{
				const auto& physics = Globals::Components().physics();
				stepF = [&, startTime = physics.simulationDuration, duration = lifeTimeRangeF().y]() {
					if (physics.simulationDuration - startTime > duration)
						state = ComponentState::Outdated;
				};
			}

			drawMode = GL_POINTS;
			bufferDataUsage = GL_DYNAMIC_COPY;
		}

		std::vector<glm::vec4> velocitiesAndTimes;
		unsigned forcedVelocitiesAndTimesCount = 0;
		std::vector<glm::vec3> hSizesAndAngles;
		unsigned forcedHSizesAndAnglesCount = 0;
		std::vector<glm::vec3> centers;
	};
}
