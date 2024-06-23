#pragma once

#include "programBase.hpp"

namespace ShadersUtils
{
	namespace Programs
	{
		struct NoiseAccessor : ProgramBase<NoiseAccessor>
		{
			using ProgramBase::ProgramBase;

			NoiseAccessor(ProgramId program):
				ProgramBase(program),
				model(program, "model"),
				vp(program, "vp"),
				color(program, "color"),
				separateNoises(program, "separateNoises"),
				noiseWeights(program, "noiseWeights"),
				noiseMin(program, "noiseMin"),
				noiseMax(program, "noiseMax"),
				time(program, "time")
			{
			}

			UniformsUtils::UniformMat4f model;
			UniformsUtils::UniformMat4f vp;
			UniformsUtils::Uniform4f color;
			UniformsUtils::Uniform1b separateNoises;
			UniformsUtils::Uniform4f noiseWeights;
			UniformsUtils::Uniform4f noiseMin;
			UniformsUtils::Uniform4f noiseMax;
			UniformsUtils::Uniform1f time;
		};

		struct Noise : NoiseAccessor
		{
			Noise():
				NoiseAccessor(LinkProgram(CompileShaders("ogl/shaders/noise.vs",
					"ogl/shaders/noise.fs"), { {0, "bPos"}, {1, "bColor"}, {4, "bInstancedTransform"} }))
			{
				model(glm::mat4(1.0f));
				vp(glm::mat4(1.0f));
				color(glm::vec4(1.0f));
				separateNoises(false);
				noiseWeights(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
				noiseMin(glm::vec4(-0.5f));
				noiseMax(glm::vec4(0.5f));
				time(0.0f);
			}

			Noise(const Noise&) = delete;

			~Noise()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
