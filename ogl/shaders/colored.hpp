#pragma once

#include <ogl/shaders.hpp>
#include <ogl/uniformControllers.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct ColoredAccessor
		{
			ColoredAccessor(Shaders::ProgramId program):
				program(program),
				modelUniform(program, "model"),
				vpUniform(program, "vp"),
				colorUniform(program, "color")
			{
			}

			Shaders::ProgramId program;
			Uniforms::UniformControllerMat4f modelUniform;
			Uniforms::UniformControllerMat4f vpUniform;
			Uniforms::UniformController4f colorUniform;
		};

		struct Colored: ColoredAccessor
		{
			Colored():
				ColoredAccessor(Shaders::LinkProgram(Shaders::CompileShaders("ogl/shaders/colored.vs",
					"ogl/shaders/colored.fs"), { {0, "bPos"}, {1, "bColor"} }))
			{
			}

			Colored(const Colored&) = delete;

			~Colored()
			{
				glDeleteProgram(program);
			}
		};
	}
}
