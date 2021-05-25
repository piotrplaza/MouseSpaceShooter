#pragma once

#include "programBase.hpp"

namespace Shaders
{
	namespace Programs
	{
		struct ColoredAccessor : ProgramBase
		{
			using ProgramBase::ProgramBase;

			ColoredAccessor(ProgramId program):
				ProgramBase(program),
				modelUniform(program, "model"),
				vpUniform(program, "vp"),
				colorUniform(program, "color")
			{
			}

			Uniforms::UniformControllerMat4f modelUniform;
			Uniforms::UniformControllerMat4f vpUniform;
			Uniforms::UniformController4f colorUniform;
		};

		struct Colored: ColoredAccessor
		{
			Colored():
				ColoredAccessor(LinkProgram(CompileShaders("ogl/shaders/colored.vs",
					"ogl/shaders/colored.fs"), { {0, "bPos"}, {1, "bColor"} }))
			{
			}

			Colored(const Colored&) = delete;

			~Colored()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
