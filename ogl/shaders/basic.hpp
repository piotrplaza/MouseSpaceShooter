#pragma once

#include "programBase.hpp"

namespace Shaders
{
	namespace Programs
	{
		struct BasicAccessor : ProgramBase<BasicAccessor>
		{
			using ProgramBase::ProgramBase;

			BasicAccessor(ProgramId program):
				ProgramBase(program),
				model(program, "model"),
				vp(program, "vp"),
				color(program, "color")
			{
			}

			Uniforms::UniformMat4f model;
			Uniforms::UniformMat4f vp;
			Uniforms::Uniform4f color;
		};

		struct Basic : BasicAccessor
		{
			Basic():
				BasicAccessor(LinkProgram(CompileShaders("ogl/shaders/basic.vs",
					"ogl/shaders/basic.fs"), { {0, "bPos"}, {1, "bColor"}, {4, "bInstancedTransform"} }))
			{
				model(glm::mat4(1.0f));
				vp(glm::mat4(1.0f));
				color(glm::vec4(1.0f));
			}

			Basic(const Basic&) = delete;

			~Basic()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
