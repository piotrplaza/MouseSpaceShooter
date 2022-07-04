#pragma once

#include "programBase.hpp"

namespace Shaders
{
	namespace Programs
	{
		struct JuliaAccessor : ProgramBase
		{
			using ProgramBase::ProgramBase;

			JuliaAccessor(ProgramId program):
				ProgramBase(program),
				vp(program, "vp"),
				juliaCOffset(program, "juliaCOffset"),
				minColor(program, "minColor"),
				maxColor(program, "maxColor")
			{
			}

			Uniforms::UniformMat4f vp;
			Uniforms::Uniform2f juliaCOffset;
			Uniforms::Uniform4f minColor;
			Uniforms::Uniform4f maxColor;
		};

		struct Julia: JuliaAccessor
		{
			Julia():
				JuliaAccessor(LinkProgram(CompileShaders("ogl/shaders/julia.vs",
					"ogl/shaders/julia.fs"), { {0, "bPos"} }))
			{
				vp(glm::mat4(1.0f));
				juliaCOffset(glm::vec2(0.0f, 0.0f));
				minColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
				maxColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			}

			Julia(const Julia&) = delete;

			~Julia()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
