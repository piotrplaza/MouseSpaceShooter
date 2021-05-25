#pragma once

#include "programBase.hpp"

namespace Shaders
{
	namespace Programs
	{
		struct TexturedColorThresholdAccessor : ProgramBase
		{
			using ProgramBase::ProgramBase;

			TexturedColorThresholdAccessor(ProgramId program):
				ProgramBase(program),
				modelUniform(program, "model"),
				vpUniform(program, "vp"),
				colorUniform(program, "color"),
				textureTranslateUniform(program, "textureTranslate"),
				textureScaleUniform(program, "textureScale"),
				texture1Uniform(program, "texture1"),
				invisibleColorUniform(program, "invisibleColor"),
				invisibleColorThresholdUniform(program, "invisibleColorThreshold")
			{
			}

			Uniforms::UniformControllerMat4f modelUniform;
			Uniforms::UniformControllerMat4f vpUniform;
			Uniforms::UniformController4f colorUniform;
			Uniforms::UniformController2f textureTranslateUniform;
			Uniforms::UniformController2f textureScaleUniform;
			Uniforms::UniformController1i texture1Uniform;
			Uniforms::UniformController3f invisibleColorUniform;
			Uniforms::UniformController1f invisibleColorThresholdUniform;
		};

		struct TexturedColorThreshold: TexturedColorThresholdAccessor
		{
			TexturedColorThreshold():
				TexturedColorThresholdAccessor(LinkProgram(CompileShaders("ogl/shaders/texturedColorThreshold.vs",
					"ogl/shaders/texturedColorThreshold.fs"), { {0, "bPos"}, {1, "bTexCoord"} }))
			{
			}

			TexturedColorThreshold(const TexturedColorThreshold&) = delete;

			~TexturedColorThreshold()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
