#pragma once

#include <ogl/shaders.hpp>
#include <ogl/uniformControllers.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct JuliaAccessor
		{
			JuliaAccessor(Shaders::ProgramId program):
				program(program),
				vpUniform(program, "vp"),
				juliaCOffsetUniform(program, "juliaCOffset"),
				minColorUniform(program, "minColor"),
				maxColorUniform(program, "maxColor")
			{
			}

			Shaders::ProgramId program;
			Uniforms::UniformControllerMat4f vpUniform;
			Uniforms::UniformController2f juliaCOffsetUniform;
			Uniforms::UniformController4f minColorUniform;
			Uniforms::UniformController4f maxColorUniform;
		};

		struct Julia: JuliaAccessor
		{
			Julia():
				JuliaAccessor(Shaders::LinkProgram(Shaders::CompileShaders("ogl/shaders/julia.vs",
					"ogl/shaders/julia.fs"), { {0, "bPos"} }))
			{
			}

			Julia(const Julia&) = delete;

			~Julia()
			{
				glDeleteProgram(program);
			}
		};
	}
}
