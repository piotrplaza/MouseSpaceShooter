#pragma once

#include <shaders.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct Basic
		{
			Basic()
			{
				program = Shaders::LinkProgram(Shaders::CompileShaders("shaders/basic.vs", "shaders/basic.fs"), { {0, "bPos"} });
				mvpUniform = glGetUniformLocation(program, "mvp");
				colorUniform = glGetUniformLocation(program, "color");
			}

			~Basic()
			{
				glDeleteProgram(program);
			}

			Shaders::ProgramId program;
			Shaders::UniformId mvpUniform;
			Shaders::UniformId colorUniform;
		};
	}
}
