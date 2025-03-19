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

		Particles(glm::vec3 sourcePoint, glm::vec3 initVelocity, glm::vec2 velocitySpreadRange, unsigned particlesCount)
		{
			positions.push_back(sourcePoint);
			forcedPositionsCount = particlesCount;
			colors.push_back(glm::vec4(1.0f));
			forcedColorsCount = particlesCount;
			velocitiesAndTimes.push_back(glm::vec4(initVelocity, 0.0f));
			forcedVelocitiesAndTimesCount = particlesCount;
			hSizesAndAngles.push_back(glm::vec3(0.0f));
			forcedHSizesAndAnglesCount = particlesCount;

			tfShaderProgram = &Globals::Shaders().tfParticles();
			tfRenderingSetupF = [&, velocitySpreadRange](auto& programBase) {
				auto& tfOrbitingParticles = static_cast<ShadersUtils::Programs::TFParticles&>(programBase);
				tfOrbitingParticles.velocitySpreadRange(velocitySpreadRange);
				return [&]() {
					tfOrbitingParticles.velocitySpreadRange(glm::vec2(0.0f));
					tfRenderingSetupF = nullptr;
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
