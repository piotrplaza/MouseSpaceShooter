#pragma once

#include <shaders.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct SceneCoordTextured
		{
			SceneCoordTextured()
			{
				program = Shaders::LinkProgram(Shaders::CompileShaders("shaders/sceneCoordTextured.vs", "shaders/sceneCoordTextured.fs"), { {0, "bPos"} });
				mvpUniform = glGetUniformLocation(program, "mvp");
				modelUniform = glGetUniformLocation(program, "model");
				textureTranslateUniform = glGetUniformLocation(program, "textureTranslate");
				textureScaleUniform = glGetUniformLocation(program, "textureScale");
				textureCoordBasedOnModelTransformUniform = glGetUniformLocation(program, "textureCoordBasedOnModelTransform");
				texture1Uniform = glGetUniformLocation(program, "texture1");
			}

			~SceneCoordTextured()
			{
				glDeleteProgram(program);
			}

			Shaders::ProgramId program;
			Shaders::UniformId mvpUniform;
			Shaders::UniformId modelUniform;
			Shaders::UniformId textureTranslateUniform;
			Shaders::UniformId textureScaleUniform;
			Shaders::UniformId textureCoordBasedOnModelTransformUniform;
			Shaders::UniformId texture1Uniform;
		};
	}
}
