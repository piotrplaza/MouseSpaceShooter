#pragma once

#include <ogl/shaders.hpp>
#include <ogl/uniformControllers.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct Julia
		{
			Julia():
				program(Shaders::LinkProgram(Shaders::CompileShaders("shaders/julia.vs", "shaders/julia.fs"), { {0, "bPos"} })),
				vpUniform(program, "vp"),
				juliaCOffsetUniform(program, "juliaCOffset"),
				minColorUniform(program, "minColorUniform"),
				maxColorUniform(program, "maxColorUniform")
			{
			}

			~Julia()
			{
				glDeleteProgram(program);
			}

			Shaders::ProgramId program;
			Uniforms::UniformControllerMat4f vpUniform;
			Uniforms::UniformController2f juliaCOffsetUniform;
			Uniforms::UniformController4f minColorUniform;
			Uniforms::UniformController4f maxColorUniform;
		};
	}
}
