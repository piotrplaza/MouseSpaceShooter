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
				textureTranslateUniform(program, "textureTranslate"),
				textureScaleUniform(program, "textureScale"),
				numOfTexturesUniform(program, "numOfTextures"),
				texturesUniform(program, "textures")
			{
			}

			Uniforms::UniformControllerMat4f modelUniform;
			Uniforms::UniformControllerMat4f vpUniform;
			Uniforms::UniformController4f colorUniform;
			Uniforms::UniformController2f textureTranslateUniform;
			Uniforms::UniformController2f textureScaleUniform;
			Uniforms::UniformController1i numOfTexturesUniform;
			Uniforms::UniformController1iv<5> texturesUniform;
		};

		struct Textured: TexturedAccessor
		{
			Textured():
				TexturedAccessor(LinkProgram(CompileShaders("ogl/shaders/textured.vs",
					"ogl/shaders/textured.fs"), { {0, "bPos"}, {1, "bTexCoord"} }))
			{
			}

			Textured(const Textured&) = delete;

			~Textured()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
