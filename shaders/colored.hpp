#pragma once

#include <ogl/shaders.hpp>
#include <ogl/uniformControllers.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct Colored
		{
			Colored():
				program(Shaders::LinkProgram(Shaders::CompileShaders("shaders/colored.vs", "shaders/colored.fs"), { {0, "bPos"}, {1, "bColor"} })),
				mvpUniform(program, "mvp"),
				colorUniform(program, "color")
			{
			}

			~Colored()
			{
				glDeleteProgram(program);
			}

			Shaders::ProgramId program;
			Uniforms::UniformControllerMat4f mvpUniform;
			Uniforms::UniformController4f colorUniform;
		};
	}
}
