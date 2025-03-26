#pragma once

#include "_componentBase.hpp"
#include "_renderable.hpp"

#include <globals/shaders.hpp>
#include <ogl/shaders/tfParticles.hpp>
#include <ogl/shaders/billboards.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>

namespace Components
{
	struct Particles : Renderable
	{
		Particles() = default;

		Particles(std::vector<glm::vec3> positions, std::vector<glm::vec4> colors, std::vector<glm::vec4> velocitiesAndTimes, std::vector<glm::vec3> hSizesAndAngles, std::vector<glm::vec3> centers = {}) :
			Renderable(std::move(positions), std::move(colors)),
			velocitiesAndTimes(std::move(velocitiesAndTimes)),
			hSizesAndAngles(std::move(hSizesAndAngles)),
			centers(std::move(centers))
		{
			tfShaderProgram = &Globals::Shaders().tfParticles();
			//customShadersProgram = &Globals::Shaders().billboards();
			drawMode = GL_POINTS;
			bufferDataUsage = GL_DYNAMIC_COPY;
		}

		Particles(FVec3 sourcePointF, FVec3 initVelocityF, FVec2 lifeTimeRangeF, std::array<FVec4, 2> colorRangeF, glm::vec2 velocitySpreadRange, float velocityRotateZHRange, glm::vec3 gravity, unsigned particlesCount)
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
				tfParticles.restart(true);
				tfParticles.origin(sourcePointF());
				tfParticles.initVelocity(initVelocityF());
				tfParticles.lifeTimeRange(lifeTimeRangeF());
				tfParticles.colorRange(0, colorRangeF[0]());
				tfParticles.colorRange(1, colorRangeF[1]());
				tfParticles.velocitySpreadFactorRange(velocitySpreadRange);
				tfParticles.velocityRotateZHRange(velocityRotateZHRange);
				tfParticles.gravity(gravity);

				return [=, &tfParticles]() {
					tfParticles.restart(false);
					tfRenderingSetupF = [=, &tfParticles](auto) {
						tfParticles.origin(sourcePointF());
						tfParticles.initVelocity(initVelocityF());
						tfParticles.lifeTimeRange(lifeTimeRangeF());
						/*tfParticles.colorRange(0, colorRangeF[0]());
						tfParticles.colorRange(1, colorRangeF[1]());*/
						tfParticles.colorRange(std::array<glm::vec4, 2>{colorRangeF[0](), colorRangeF[1]()});
						return nullptr;
					};
				};
			};

			drawMode = GL_POINTS;
			bufferDataUsage = GL_DYNAMIC_DRAW;
		}

		std::vector<glm::vec4> velocitiesAndTimes;
		unsigned forcedVelocitiesAndTimesCount = 0;
		std::vector<glm::vec3> hSizesAndAngles;
		unsigned forcedHSizesAndAnglesCount = 0;
		std::vector<glm::vec3> centers;
	};
}
