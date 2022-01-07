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
				vpUniform(program, "vp"),
				colorUniform(program, "color"),
				texture1Uniform(program, "texture1")
			{
			}

			Uniforms::UniformControllerMat4f vpUniform;
			Uniforms::UniformController4f colorUniform;
			Uniforms::UniformController1i texture1Uniform;
		};

		struct Particles : ParticlesAccessor
		{
			Particles() :
				ParticlesAccessor(LinkProgram(CompileShaders("ogl/shaders/particles.vs",
					"ogl/shaders/particles.gs", "ogl/shaders/particles.fs"), { {0, "bPos"} }))
			{
				vpUniform(glm::mat4(1.0f));
				colorUniform(glm::vec4(1.0f));
			}

			Particles(const Particles&) = delete;

			~Particles()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
