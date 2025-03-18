#pragma once

#include "_componentBase.hpp"
#include "_renderable.hpp"

#include <globals/shaders.hpp>
#include <ogl/shaders/tfOrbitingParticles.hpp>
#include <ogl/shaders/particles.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>

namespace Components
{
	struct Particles : Renderable
	{
		Particles() = default;
		Particles(std::vector<glm::vec3> vertices, std::vector<glm::vec4> colors, std::vector<glm::vec4> velocitiesAndTimes, std::vector<glm::vec3> hSizesAndAngles, std::vector<glm::vec3> centers = {}) :
			Renderable(std::move(vertices), std::move(colors)),
			velocitiesAndTimes(std::move(velocitiesAndTimes)),
			hSizesAndAngles(std::move(hSizesAndAngles)),
			centers(std::move(centers))
		{
			tfShaderProgram = &Globals::Shaders().tfOrbitingParticles();
			//customShadersProgram = &Globals::Shaders().particles();
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
