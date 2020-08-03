#pragma once

#include <ogl/shaders.hpp>
#include <ogl/uniformControllers.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct TexturedColorThresholdAccessor
		{
			TexturedColorThresholdAccessor(Shaders::ProgramId program):
				program(program),
				modelUniform(program, "model"),
				vpUniform(program, "vp"),
				colorUniform(program, "color"),
				textureTranslateUniform(program, "textureTranslate"),
				textureScaleUniform(program, "textureScale"),
				texture1Uniform(program, "texture1")
			{
			}

			Shaders::ProgramId program;
			Uniforms::UniformControllerMat4f modelUniform;
			Uniforms::UniformControllerMat4f vpUniform;
			Uniforms::UniformController4f colorUniform;
			Uniforms::UniformController2f textureTranslateUniform;
			Uniforms::UniformController2f textureScaleUniform;
			Uniforms::UniformController1i texture1Uniform;
		};

		struct TexturedColorThreshold: TexturedColorThresholdAccessor
		{
			TexturedColorThreshold():
				TexturedColorThresholdAccessor(Shaders::LinkProgram(Shaders::CompileShaders("ogl/shaders/texturedColorThreshold.vs",
					"ogl/shaders/texturedColorThreshold.fs"), { {0, "bPos"}, {1, "bTexCoord"} }))
			{
			}

			TexturedColorThreshold(const TexturedColorThreshold&) = delete;

			~TexturedColorThreshold()
			{
				glDeleteProgram(program);
			}
		};
	}
}
