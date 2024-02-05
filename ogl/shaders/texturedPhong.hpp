#pragma once

#include "programBase.hpp"

namespace Shaders
{
	namespace Programs
	{
		struct TexturedPhongAccessor : ProgramBase<TexturedPhongAccessor>
		{
			using ProgramBase::ProgramBase;

			TexturedPhongAccessor(ProgramId program):
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
				lightModelEnabled(program, "lightModelEnabled"),
				mulBlendingColor(program, "mulBlendingColor"),
				addBlendingColor(program, "addBlendingColor"),
				numOfTextures(program, "numOfTextures"),
				textures(program, "textures"),
				texturesBaseTransform(program, "texturesBaseTransform"),
				texturesCustomTransform(program, "texturesCustomTransform"),
				alphaFromBlendingTexture(program, "alphaFromBlendingTexture"),
				colorAccumulation(program, "colorAccumulation")
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
			Uniforms::Uniform1b lightModelEnabled;
			Uniforms::Uniform4f mulBlendingColor;
			Uniforms::Uniform4f addBlendingColor;
			Uniforms::Uniform1i numOfTextures;
			Uniforms::Uniform1iv<5> textures;
			Uniforms::UniformMat4fv<5> texturesBaseTransform;
			Uniforms::UniformMat4fv<5> texturesCustomTransform;
			Uniforms::Uniform1b alphaFromBlendingTexture;
			Uniforms::Uniform1b colorAccumulation;
		};

		struct TexturedPhong : TexturedPhongAccessor
		{
			TexturedPhong():
				TexturedPhongAccessor(LinkProgram(CompileShaders("ogl/shaders/texturedPhong.vs",
					"ogl/shaders/texturedPhong.fs"), { {0, "bPos"}, {1, "bColor"}, {2, "bTexCoord"}, {3, "bNormal"} }))
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
				lightModelEnabled(true);
				mulBlendingColor(glm::vec4(1.0f));
				addBlendingColor(glm::vec4(0.0f));
				numOfTextures(1);
				texturesBaseTransform(glm::mat4(1.0f));
				texturesCustomTransform(glm::mat4(1.0f));
				alphaFromBlendingTexture(false);
				colorAccumulation(false);
			}

			TexturedPhong(const TexturedPhong&) = delete;

			~TexturedPhong()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
