#pragma once

#include "programBase.hpp"

namespace ShadersUtils
{
	namespace Programs
	{
		struct ParticlesAccessor : ProgramBase<ParticlesAccessor>
		{
			using ProgramBase::ProgramBase;

			ParticlesAccessor(ProgramId program):
				ProgramBase(program),
				vp(program, "vp"),
				color(program, "color"),
				texture0(program, "texture0")
			{
			}

			UniformsUtils::UniformMat4f vp;
			UniformsUtils::Uniform4f color;
			UniformsUtils::Uniform1i texture0;
		};

		struct Particles : ParticlesAccessor
		{
			Particles() :
				ParticlesAccessor(LinkProgram(CompileShaders("ogl/shaders/particles.vs",
					"ogl/shaders/particles.gs", "ogl/shaders/particles.fs"), { {0, "bPos"}, {1, "bColor"} }))
			{
				vp(glm::mat4(1.0f));
				color(glm::vec4(1.0f));
			}

			Particles(const Particles&) = delete;

			~Particles()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
