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
				program(Shaders::LinkProgram(Shaders::CompileShaders("ogl/shaders/sceneCoordTextured.vs", "ogl/shaders/sceneCoordTextured.fs"), { {0, "bPos"} })),
				modelUniform(program, "model"),
				vpUniform(program, "vp"),
				colorUniform(program, "color"),
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
			Uniforms::UniformControllerMat4f modelUniform;
			Uniforms::UniformControllerMat4f vpUniform;
			Uniforms::UniformController4f colorUniform;
			Uniforms::UniformController2f textureTranslateUniform;
			Uniforms::UniformController2f textureScaleUniform;
			Uniforms::UniformController1i textureCoordBasedOnModelTransformUniform;
			Uniforms::UniformController1i texture1Uniform;
		};
	}
}
