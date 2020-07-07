#pragma once

#include <shaders.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct Textured
		{
			Textured()
			{
				program = Shaders::LinkProgram(Shaders::CompileShaders("shaders/sceneCoordTextured.vs", "shaders/sceneCoordTextured.fs"), { {0, "bPos"}, {1, "bTexCoord"} });
				mvpUniform = glGetUniformLocation(program, "mvp");
				textureTranslateUniform = glGetUniformLocation(program, "textureTranslate");
				textureScaleUniform = glGetUniformLocation(program, "textureScale");
				texture1Uniform = glGetUniformLocation(program, "texture1");
			}

			~Textured()
			{
				glDeleteProgram(program);
			}

			Shaders::ProgramId program;
			Shaders::UniformId mvpUniform;
			Shaders::UniformId textureTranslateUniform;
			Shaders::UniformId textureScaleUniform;
			Shaders::UniformId texture1Uniform;
		};
	}
}
