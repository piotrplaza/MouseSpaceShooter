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
				program(Shaders::LinkProgram(Shaders::CompileShaders("ogl/shaders/colored.vs", "ogl/shaders/colored.fs"), { {0, "bPos"}, {1, "bColor"} })),
				modelUniform(program, "model"),
				vpUniform(program, "vp"),
				colorUniform(program, "color")
			{
			}

			~Colored()
			{
				glDeleteProgram(program);
			}

			Shaders::ProgramId program;
			Uniforms::UniformControllerMat4f modelUniform;
			Uniforms::UniformControllerMat4f vpUniform;
			Uniforms::UniformController4f colorUniform;
		};
	}
}
