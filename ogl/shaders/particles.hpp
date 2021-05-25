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
				textureTranslateUniform(program, "textureTranslate"),
				textureScaleUniform(program, "textureScale"),
				texture1Uniform(program, "texture1")
			{
			}

			Uniforms::UniformControllerMat4f vpUniform;
			Uniforms::UniformController4f colorUniform;
			Uniforms::UniformController2f textureTranslateUniform;
			Uniforms::UniformController2f textureScaleUniform;
			Uniforms::UniformController1i texture1Uniform;
		};

		struct Particles : ParticlesAccessor
		{
			Particles() :
				ParticlesAccessor(LinkProgram(CompileShaders("ogl/shaders/particles.vs",
					"ogl/shaders/particles.gs", "ogl/shaders/particles.fs"), { {0, "bPos"} }))
			{
			}

			Particles(const Particles&) = delete;

			~Particles()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
