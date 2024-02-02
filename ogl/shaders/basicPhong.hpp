#pragma once

#include "programBase.hpp"

namespace Shaders
{
	namespace Programs
	{
		struct BasicPhongAccessor : ProgramBase<BasicPhongAccessor>
		{
			using ProgramBase::ProgramBase;

			BasicPhongAccessor(ProgramId program):
				ProgramBase(program),
				model(program, "model"),
				vp(program, "vp"),
				normalMatrix(program, "normalMatrix"),
				color(program, "color"),
				clearColor(program, "clearColor"),
				numOfLights(program, "numOfLights"),
				lightsPos(program, "lightsPos"),
				lightsCol(program, "lightsCol"),
				lightsAttenuation(program, "lightsAttenuation"),
				ambient(program, "ambient"),
				diffuse(program, "diffuse"),
				viewPos(program, "viewPos"),
				specular(program, "specular"),
				specularFocus(program, "specularFocus"),
				flatColor(program, "flatColor"),
				flatNormal(program, "flatNormal"),
				lightModelColorNormalization(program, "lightModelColorNormalization")
			{
			}

			Uniforms::UniformMat4f model;
			Uniforms::UniformMat4f vp;
			Uniforms::UniformMat3f normalMatrix;
			Uniforms::Uniform4f color;
			Uniforms::Uniform3f clearColor;
			Uniforms::Uniform1i numOfLights;
			Uniforms::Uniform3fv<128> lightsPos;
			Uniforms::Uniform3fv<128> lightsCol;
			Uniforms::Uniform1fv<128> lightsAttenuation;
			Uniforms::Uniform1f ambient;
			Uniforms::Uniform1f diffuse;
			Uniforms::Uniform3f viewPos;
			Uniforms::Uniform1f specular;
			Uniforms::Uniform1f specularFocus;
			Uniforms::Uniform1b flatColor;
			Uniforms::Uniform1b flatNormal;
			Uniforms::Uniform1b lightModelColorNormalization;
		};

		struct BasicPhong : BasicPhongAccessor
		{
			BasicPhong():
				BasicPhongAccessor(LinkProgram(CompileShaders("ogl/shaders/basicPhong.vs",
					"ogl/shaders/basicPhong.fs"), { {0, "bPos"}, {1, "bColor"}, {3, "bNormal"} }))
			{
				model(glm::mat4(1.0f));
				vp(glm::mat4(1.0f));
				normalMatrix(glm::mat3(1.0f));
				color(glm::vec4(1.0f));
				clearColor(glm::vec3(0.0f));
				numOfLights(0);
				lightsPos(glm::vec3(0.0f));
				lightsCol(glm::vec3(1.0f));
				lightsAttenuation(0.0f);
				ambient(1.0f);
				diffuse(0.0f);
				viewPos(glm::vec3(0.0f));
				specular(0.0f);
				specularFocus(16.0f);
				flatColor(false);
				flatNormal(false);
				lightModelColorNormalization(false);
			}

			BasicPhong(const BasicPhong&) = delete;

			~BasicPhong()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
