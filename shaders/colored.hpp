#pragma once

#include <shaders.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct Colored
		{
			Colored()
			{
				program = Shaders::LinkProgram(Shaders::CompileShaders("shaders/colored.vs", "shaders/colored.fs"), { {0, "bPos"}, {1, "bColor"} });
				mvpUniform = glGetUniformLocation(program, "mvp");
			}

			~Colored()
			{
				glDeleteProgram(program);
			}

			Shaders::ProgramId program;
			Shaders::UniformId mvpUniform;
		};
	}
}
