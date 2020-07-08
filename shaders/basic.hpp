#pragma once

#include <ogl/shaders.hpp>
#include <ogl/uniformControllers.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct Basic
		{
			Basic():
				program(Shaders::LinkProgram(Shaders::CompileShaders("shaders/basic.vs", "shaders/basic.fs"), { {0, "bPos"} })),
				mvpUniform(program, "mvp"),
				colorUniform(program, "color")
			{
			}

			~Basic()
			{
				glDeleteProgram(program);
			}

			Shaders::ProgramId program;
			Uniforms::UniformControllerMat4f mvpUniform;
			Uniforms::UniformController4f colorUniform;
		};
	}
}
