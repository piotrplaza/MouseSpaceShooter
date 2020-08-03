#pragma once

#include <ogl/shaders.hpp>
#include <ogl/uniformControllers.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct BasicAccessor
		{
			BasicAccessor(Shaders::ProgramId program) :
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

		struct Basic: BasicAccessor
		{
			Basic():
				BasicAccessor(Shaders::LinkProgram(Shaders::CompileShaders("ogl/shaders/basic.vs",
					"ogl/shaders/basic.fs"), { {0, "bPos"} }))
			{
			}

			Basic(const Basic&) = delete;

			~Basic()
			{
				glDeleteProgram(program);
			}
		};
	}
}
