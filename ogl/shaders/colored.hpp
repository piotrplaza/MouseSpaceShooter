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
				model(program, "model"),
				vp(program, "vp"),
				color(program, "color")
			{
			}

			Uniforms::UniformControllerMat4f model;
			Uniforms::UniformControllerMat4f vp;
			Uniforms::UniformController4f color;
		};

		struct Colored: ColoredAccessor
		{
			Colored():
				ColoredAccessor(LinkProgram(CompileShaders("ogl/shaders/colored.vs",
					"ogl/shaders/colored.fs"), { {0, "bPos"}, {1, "bColor"} }))
			{
				model(glm::mat4(1.0f));
				vp(glm::mat4(1.0f));
				color(glm::vec4(1.0f));
			}

			Colored(const Colored&) = delete;

			~Colored()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
