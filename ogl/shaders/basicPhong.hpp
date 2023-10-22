#pragma once

#include "programBase.hpp"

namespace Shaders
{
	namespace Programs
	{
		struct BasicPhongAccessor : ProgramBase
		{
			using ProgramBase::ProgramBase;

			BasicPhongAccessor(ProgramId program):
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

		struct BasicPhong : BasicPhongAccessor
		{
			BasicPhong():
				BasicPhongAccessor(LinkProgram(CompileShaders("ogl/shaders/basicPhong.vs",
					"ogl/shaders/basicPhong.fs"), { {0, "bPos"}, {1, "bColor"}, {2, "bNormal"} }))
			{
				model(glm::mat4(1.0f));
				vp(glm::mat4(1.0f));
				color(glm::vec4(1.0f));
			}

			BasicPhong(const BasicPhong&) = delete;

			~BasicPhong()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
