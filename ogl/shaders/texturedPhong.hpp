#pragma once

#include "programBase.hpp"

namespace Shaders
{
	namespace Programs
	{
		struct TexturedPhongAccessor : ProgramBase
		{
			using ProgramBase::ProgramBase;

			TexturedPhongAccessor(ProgramId program):
				ProgramBase(program),
				model(program, "model"),
				vp(program, "vp"),
				color(program, "color"),
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
			Uniforms::Uniform4f color;
			Uniforms::Uniform4f mulBlendingColor;
			Uniforms::Uniform4f addBlendingColor;
			Uniforms::Uniform1i numOfTextures;
			Uniforms::Uniform1iv<5> textures;
			Uniforms::UniformMat4fv<5> texturesBaseTransform;
			Uniforms::UniformMat4fv<5> texturesCustomTransform;
			Uniforms::Uniform1b alphaFromBlendingTexture;
			Uniforms::Uniform1b colorAccumulation;
		};

		struct TexturedPhong: TexturedPhongAccessor
		{
			TexturedPhong():
				TexturedPhongAccessor(LinkProgram(CompileShaders("ogl/shaders/texturedPhong.vs",
					"ogl/shaders/texturedPhong.fs"), { {0, "bPos"}, {1, "bColor"}, {2, "bTexCoord"}, {3, "bNormal"} }))
			{
				model(glm::mat4(1.0f));
				vp(glm::mat4(1.0f));
				color(glm::vec4(1.0f));
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
