#pragma once

#include "programBase.hpp"

namespace Shaders
{
	namespace Programs
	{
		struct TexturedAccessor : ProgramBase
		{
			using ProgramBase::ProgramBase;

			TexturedAccessor(ProgramId program):
				ProgramBase(program),
				modelUniform(program, "model"),
				vpUniform(program, "vp"),
				colorUniform(program, "color"),
				numOfTexturesUniform(program, "numOfTextures"),
				texturesUniform(program, "textures"),
				texturesTranslateUniform(program, "texturesTranslate"),
				texturesScaleUniform(program, "texturesScale")
			{
			}

			Uniforms::UniformControllerMat4f modelUniform;
			Uniforms::UniformControllerMat4f vpUniform;
			Uniforms::UniformController4f colorUniform;
			Uniforms::UniformController1i numOfTexturesUniform;
			Uniforms::UniformController1iv<5> texturesUniform;
			Uniforms::UniformController2fv<5> texturesTranslateUniform;
			Uniforms::UniformController2fv<5> texturesScaleUniform;
		};

		struct Textured: TexturedAccessor
		{
			Textured():
				TexturedAccessor(LinkProgram(CompileShaders("ogl/shaders/textured.vs",
					"ogl/shaders/textured.fs"), { {0, "bPos"}, {1, "bTexCoord"} }))
			{
				modelUniform(glm::mat4(1.0f));
				vpUniform(glm::mat4(1.0f));
				colorUniform(glm::vec4(1.0f));
				//numOfTexturesUniform(1);
				texturesTranslateUniform(glm::vec2(0.0f));
				texturesScaleUniform(glm::vec2(1.0f));
			}

			Textured(const Textured&) = delete;

			~Textured()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
