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
				blendingColorUniform(program, "blendingColor"),
				numOfTexturesUniform(program, "numOfTextures"),
				texturesUniform(program, "textures"),
				texturesTranslateUniform(program, "texturesTranslate"),
				texturesScaleUniform(program, "texturesScale"),
				invisibleColorUniform(program, "invisibleColor"),
				invisibleColorThresholdUniform(program, "invisibleColorThreshold")
			{
			}

			Uniforms::UniformControllerMat4f modelUniform;
			Uniforms::UniformControllerMat4f vpUniform;
			Uniforms::UniformController4f colorUniform;
			Uniforms::UniformController4f blendingColorUniform;
			Uniforms::UniformController1i numOfTexturesUniform;
			Uniforms::UniformController1iv<5> texturesUniform;
			Uniforms::UniformController2fv<5> texturesTranslateUniform;
			Uniforms::UniformController2fv<5> texturesScaleUniform;
			Uniforms::UniformController3f invisibleColorUniform;
			Uniforms::UniformController1f invisibleColorThresholdUniform;
		};

		struct TexturedColorThreshold: TexturedColorThresholdAccessor
		{
			TexturedColorThreshold():
				TexturedColorThresholdAccessor(LinkProgram(CompileShaders("ogl/shaders/texturedColorThreshold.vs",
					"ogl/shaders/texturedColorThreshold.fs"), { {0, "bPos"}, {1, "bTexCoord"} }))
			{
				modelUniform(glm::mat4(1.0f));
				vpUniform(glm::mat4(1.0f));
				colorUniform(glm::vec4(1.0f));
				blendingColorUniform(glm::vec4(1.0f));
				numOfTexturesUniform(1);
				texturesTranslateUniform(glm::vec2(0.0f));
				texturesScaleUniform(glm::vec2(1.0f));
				invisibleColorUniform(glm::vec3(0.0f));
				invisibleColorThresholdUniform(0.0f);
			}

			TexturedColorThreshold(const TexturedColorThreshold&) = delete;

			~TexturedColorThreshold()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
