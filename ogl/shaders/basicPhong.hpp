#pragma once

#include "programBase.hpp"

namespace ShadersUtils
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
				lightsDarkColorFactor(program, "lightsDarkColorFactor"),
				ambient(program, "ambient"),
				diffuse(program, "diffuse"),
				viewPos(program, "viewPos"),
				specular(program, "specular"),
				specularFocus(program, "specularFocus"),
				specularMaterialColorFactor(program, "specularMaterialColorFactor"),
				illumination(program, "illumination"),
				darkColor(program, "darkColor"),
				flatColor(program, "flatColor"),
				flatNormal(program, "flatNormal"),
				lightModelColorNormalization(program, "lightModelColorNormalization"),
				lightModelEnabled(program, "lightModelEnabled"),
				gpuSideInstancedNormalTransforms(program, "gpuSideInstancedNormalTransforms"),
				fogAmplification(program, "fogAmplification")
			{
			}

			UniformsUtils::UniformMat4f model;
			UniformsUtils::UniformMat4f vp;
			UniformsUtils::UniformMat3f normalMatrix;
			UniformsUtils::Uniform4f color;
			UniformsUtils::Uniform3f clearColor;
			UniformsUtils::Uniform1i numOfLights;
			UniformsUtils::Uniform3fv<128> lightsPos;
			UniformsUtils::Uniform3fv<128> lightsCol;
			UniformsUtils::Uniform1fv<128> lightsAttenuation;
			UniformsUtils::Uniform1fv<128> lightsDarkColorFactor;
			UniformsUtils::Uniform1f ambient;
			UniformsUtils::Uniform1f diffuse;
			UniformsUtils::Uniform3f viewPos;
			UniformsUtils::Uniform1f specular;
			UniformsUtils::Uniform1f specularFocus;
			UniformsUtils::Uniform1f specularMaterialColorFactor;
			UniformsUtils::Uniform4f illumination;
			UniformsUtils::Uniform3f darkColor;
			UniformsUtils::Uniform1b flatColor;
			UniformsUtils::Uniform1b flatNormal;
			UniformsUtils::Uniform1b lightModelColorNormalization;
			UniformsUtils::Uniform1b lightModelEnabled;
			UniformsUtils::Uniform1b gpuSideInstancedNormalTransforms;
			UniformsUtils::Uniform1f fogAmplification;
		};

		struct BasicPhong : BasicPhongAccessor
		{
			BasicPhong():
				BasicPhongAccessor(LinkProgram(CompileShaders("ogl/shaders/basicPhong.vs",
					"ogl/shaders/basicPhong.fs"), { {0, "bPos"}, {1, "bColor"}, {3, "bNormal"}, {4, "bInstancedTransform"}, {8, "bInstancedNormalTransform"} }))
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
				lightsDarkColorFactor(0.0f);
				ambient(1.0f);
				diffuse(0.0f);
				viewPos(glm::vec3(0.0f));
				specular(0.0f);
				specularFocus(16.0f);
				specularMaterialColorFactor(0.0f);
				illumination(glm::vec4(0.0f));
				darkColor(glm::vec3(0.0f));
				flatColor(false);
				flatNormal(false);
				lightModelColorNormalization(false);
				lightModelEnabled(true);
				gpuSideInstancedNormalTransforms(false);
				fogAmplification(0.0f);
			}

			BasicPhong(const BasicPhong&) = delete;

			~BasicPhong()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
