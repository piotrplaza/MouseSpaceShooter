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
#include <variant>

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
				tfParticles.componentId(getComponentId());
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

		Particles(std::variant<FVec3, std::pair<FVec3, FVec3>> sourceFV, FVec3 initVelocityF, FVec2 lifeTimeRangeF, std::array<FVec4, 2> colorRangeF, glm::vec2 velocitySpreadFactorRange,
			float velocityRotateZHRange, FVec3 globalForceF, bool respawning, unsigned particlesCount)
		{
			auto unifiedSourceFV = std::visit([&](auto&& source) {
				using T = std::decay_t<decltype(source)>;
				if constexpr (std::is_same_v<T, FVec3>)
					return std::make_pair(source, source);
				else if constexpr (std::is_same_v<T, std::pair<FVec3, FVec3>>)
					return std::make_pair(source.first, source.second);
				else
					static_assert("Invalid source type");
				}, sourceFV);

			positions.push_back(unifiedSourceFV.second());
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
				tfParticles.componentId(getComponentId());
				tfParticles.particlesCount(particlesCount);
				tfParticles.originBegin(unifiedSourceFV.first());
				tfParticles.originEnd(unifiedSourceFV.second());
				tfParticles.initVelocity(initVelocityF());
				tfParticles.lifeTimeRange(lifeTimeRangeF());
				tfParticles.colorRange({colorRangeF[0](), colorRangeF[1]()});
				tfParticles.velocitySpreadFactorRange(velocitySpreadFactorRange);
				tfParticles.velocityRotateZHRange(velocityRotateZHRange);
				tfParticles.globalForce(globalForceF());
				tfParticles.respawning(respawning);

				return [=, &tfParticles]() {
					tfParticles.init(false);
					tfRenderingSetupF = [=, &tfParticles](auto) {
						tfParticles.originBegin(unifiedSourceFV.first());
						tfParticles.originEnd(unifiedSourceFV.second());
						tfParticles.initVelocity(initVelocityF());
						tfParticles.lifeTimeRange(lifeTimeRangeF());
						tfParticles.colorRange({colorRangeF[0](), colorRangeF[1]()});
						tfParticles.globalForce(globalForceF());
						return nullptr;
					};
				};
			};

			if (auto duration = lifeTimeRangeF().y; duration != 0.0f && !respawning)
			{
				const auto& physics = Globals::Components().physics();
				stepF = [&, startTime = physics.simulationDuration, duration]() {
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
	};
}
