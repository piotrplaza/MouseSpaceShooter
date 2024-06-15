#pragma once

#include "programBase.hpp"

namespace Shaders
{
	namespace Programs
	{
		struct JuliaAccessor : ProgramBase<JuliaAccessor>
		{
			using ProgramBase::ProgramBase;

			JuliaAccessor(ProgramId program):
				ProgramBase(program),
				vp(program, "vp"),
				juliaC(program, "juliaC"),
				juliaCOffset(program, "juliaCOffset"),
				minColor(program, "minColor"),
				maxColor(program, "maxColor"),
				zoom(program, "zoom"),
				translation(program, "translation"),
				iterations(program, "iterations")
			{
			}

			Uniforms::UniformMat4f vp;
			Uniforms::Uniform2f juliaC;
			Uniforms::Uniform2f juliaCOffset;
			Uniforms::Uniform4f minColor;
			Uniforms::Uniform4f maxColor;
			Uniforms::Uniform1f zoom;
			Uniforms::Uniform2f translation;
			Uniforms::Uniform1i iterations;
		};

		struct Julia : JuliaAccessor
		{
			Julia():
				JuliaAccessor(LinkProgram(CompileShaders("ogl/shaders/julia.vs",
					"ogl/shaders/julia.fs"), { {0, "bPos"} }))
			{
				vp(glm::mat4(1.0f));
				juliaC({ -0.1f, 0.65f });
				juliaCOffset(glm::vec2(0.0f));
				minColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
				maxColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				zoom(1.0f);
				translation(glm::vec2(0.0f));
				iterations(100);
			}

			Julia(const Julia&) = delete;

			~Julia()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
