#pragma once

#include <ogl/shaders.hpp>
#include <ogl/uniformControllers.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct Textured
		{
			Textured() :
				program(Shaders::LinkProgram(Shaders::CompileShaders("shaders/textured.vs", "shaders/textured.fs"), { {0, "bPos"}, {1, "bTexCoord"} })),
				mvpUniform(program, "mvp"),
				colorUniform(program, "color"),
				textureTranslateUniform(program, "textureTranslate"),
				textureScaleUniform(program, "textureScale"),
				texture1Uniform(program, "texture1")
			{
			}

			~Textured()
			{
				glDeleteProgram(program);
			}

			Shaders::ProgramId program;
			Uniforms::UniformControllerMat4f mvpUniform;
			Uniforms::UniformController4f colorUniform;
			Uniforms::UniformController2f textureTranslateUniform;
			Uniforms::UniformController2f textureScaleUniform;
			Uniforms::UniformController1i texture1Uniform;
		};
	}
}
