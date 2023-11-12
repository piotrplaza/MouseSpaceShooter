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
				mvr(program, "mvr"),
				color(program, "color"),
				numOfLights(program, "numOfLights"),
				lightsPos(program, "lightsPos"),
				lightsCol(program, "lightsCol")
			{
			}

			Uniforms::UniformMat4f model;
			Uniforms::UniformMat4f vp;
			Uniforms::UniformMat3f mvr;
			Uniforms::Uniform4f color;
			Uniforms::Uniform1i numOfLights;
			Uniforms::Uniform3fv<128> lightsPos;
			Uniforms::Uniform3fv<128> lightsCol;
		};

		struct BasicPhong : BasicPhongAccessor
		{
			BasicPhong():
				BasicPhongAccessor(LinkProgram(CompileShaders("ogl/shaders/basicPhong.vs",
					"ogl/shaders/basicPhong.fs"), { {0, "bPos"}, {1, "bColor"}, {2, "bNormal"} }))
			{
				model(glm::mat4(1.0f));
				vp(glm::mat4(1.0f));
				mvr(glm::mat3(1.0f));
				color(glm::vec4(1.0f));
				numOfLights(0);
				lightsPos(glm::vec3(0.0f));
				lightsCol(glm::vec3(1.0f));
			}

			BasicPhong(const BasicPhong&) = delete;

			~BasicPhong()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
