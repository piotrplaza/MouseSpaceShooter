#pragma once

#include "programBase.hpp"

namespace Shaders
{
	namespace Programs
	{
		struct ParticlesAccessor : ProgramBase
		{
			using ProgramBase::ProgramBase;

			ParticlesAccessor(ProgramId program) :
				ProgramBase(program),
				vp(program, "vp"),
				color(program, "color"),
				texture1(program, "texture1")
			{
			}

			Uniforms::UniformMat4f vp;
			Uniforms::Uniform4f color;
			Uniforms::Uniform1i texture1;
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
