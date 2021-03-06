#pragma once

#include "programBase.hpp"

namespace Shaders
{
	namespace Programs
	{
		struct BasicAccessor : ProgramBase
		{
			using ProgramBase::ProgramBase;

			BasicAccessor(ProgramId program) :
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

		struct Basic: BasicAccessor
		{
			Basic():
				BasicAccessor(LinkProgram(CompileShaders("ogl/shaders/basic.vs",
					"ogl/shaders/basic.fs"), { {0, "bPos"} }))
			{
			}

			Basic(const Basic&) = delete;

			~Basic()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
