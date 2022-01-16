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

			Uniforms::UniformControllerMat4f vp;
			Uniforms::UniformController4f color;
			Uniforms::UniformController1i texture1;
		};

		struct Particles : ParticlesAccessor
		{
			Particles() :
				ParticlesAccessor(LinkProgram(CompileShaders("ogl/shaders/particles.vs",
					"ogl/shaders/particles.gs", "ogl/shaders/particles.fs"), { {0, "bPos"} }))
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
