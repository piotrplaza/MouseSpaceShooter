#pragma once

#include "programBase.hpp"

namespace ShadersUtils
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

			UniformsUtils::UniformMat4f vp;
			UniformsUtils::Uniform2f juliaC;
			UniformsUtils::Uniform2f juliaCOffset;
			UniformsUtils::Uniform4f minColor;
			UniformsUtils::Uniform4f maxColor;
			UniformsUtils::Uniform1f zoom;
			UniformsUtils::Uniform2f translation;
			UniformsUtils::Uniform1i iterations;
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
