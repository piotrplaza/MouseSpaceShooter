#pragma once

#include <ogl/shaders.hpp>
#include <ogl/uniformControllers.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct SceneCoordTextured
		{
			SceneCoordTextured():
				program(Shaders::LinkProgram(Shaders::CompileShaders("shaders/sceneCoordTextured.vs", "shaders/sceneCoordTextured.fs"), { {0, "bPos"} })),
				mvpUniform(program, "mvp"),
				modelUniform(program, "model"),
				textureTranslateUniform(program, "textureTranslate"),
				textureScaleUniform(program, "textureScale"),
				textureCoordBasedOnModelTransformUniform(program, "textureCoordBasedOnModelTransform"),
				texture1Uniform(program, "texture1")
			{
			}

			~SceneCoordTextured()
			{
				glDeleteProgram(program);
			}

			Shaders::ProgramId program;
			Uniforms::UniformControllerMat4f mvpUniform;
			Uniforms::UniformControllerMat4f modelUniform;
			Uniforms::UniformController2f textureTranslateUniform;
			Uniforms::UniformController2f textureScaleUniform;
			Uniforms::UniformController1i textureCoordBasedOnModelTransformUniform;
			Uniforms::UniformController1i texture1Uniform;
		};
	}
}