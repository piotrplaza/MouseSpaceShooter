#pragma once

#include "programBase.hpp"

namespace Shaders
{
	namespace Programs
	{
		struct JuliaAccessor : ProgramBase
		{
			using ProgramBase::ProgramBase;

			JuliaAccessor(ProgramId program):
				ProgramBase(program),
				vpUniform(program, "vp"),
				juliaCOffsetUniform(program, "juliaCOffset"),
				minColorUniform(program, "minColor"),
				maxColorUniform(program, "maxColor")
			{
			}

			Uniforms::UniformControllerMat4f vpUniform;
			Uniforms::UniformController2f juliaCOffsetUniform;
			Uniforms::UniformController4f minColorUniform;
			Uniforms::UniformController4f maxColorUniform;
		};

		struct Julia: JuliaAccessor
		{
			Julia():
				JuliaAccessor(LinkProgram(CompileShaders("ogl/shaders/julia.vs",
					"ogl/shaders/julia.fs"), { {0, "bPos"} }))
			{
				vpUniform(glm::mat4(1.0f));
				juliaCOffsetUniform(glm::vec2(0.0f, 0.0f));
				minColorUniform(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
				maxColorUniform(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			}

			Julia(const Julia&) = delete;

			~Julia()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
