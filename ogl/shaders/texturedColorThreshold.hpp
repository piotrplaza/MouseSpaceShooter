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
				model(program, "model"),
				vp(program, "vp"),
				color(program, "color"),
				mulBlendingColor(program, "mulBlendingColor"),
				addBlendingColor(program, "addBlendingColor"),
				numOfTextures(program, "numOfTextures"),
				textures(program, "textures"),
				texturesTranslate(program, "texturesTranslate"),
				texturesScale(program, "texturesScale"),
				alphaFromBlendingTexture(program, "alphaFromBlendingTexture"),
				colorAccumulation(program, "colorAccumulation"),
				invisibleColor(program, "invisibleColor"),
				invisibleColorThreshold(program, "invisibleColorThreshold")
			{
			}

			Uniforms::UniformControllerMat4f model;
			Uniforms::UniformControllerMat4f vp;
			Uniforms::UniformController4f color;
			Uniforms::UniformController4f mulBlendingColor;
			Uniforms::UniformController4f addBlendingColor;
			Uniforms::UniformController1i numOfTextures;
			Uniforms::UniformController1iv<5> textures;
			Uniforms::UniformController2fv<5> texturesTranslate;
			Uniforms::UniformController2fv<5> texturesScale;
			Uniforms::UniformController1b alphaFromBlendingTexture;
			Uniforms::UniformController1b colorAccumulation;
			Uniforms::UniformController3f invisibleColor;
			Uniforms::UniformController1f invisibleColorThreshold;
		};

		struct TexturedColorThreshold: TexturedColorThresholdAccessor
		{
			TexturedColorThreshold():
				TexturedColorThresholdAccessor(LinkProgram(CompileShaders("ogl/shaders/texturedColorThreshold.vs",
					"ogl/shaders/texturedColorThreshold.fs"), { {0, "bPos"}, {1, "bTexCoord"} }))
			{
				model(glm::mat4(1.0f));
				vp(glm::mat4(1.0f));
				color(glm::vec4(1.0f));
				mulBlendingColor(glm::vec4(1.0f));
				addBlendingColor(glm::vec4(0.0f));
				numOfTextures(1);
				texturesTranslate(glm::vec2(0.0f));
				texturesScale(glm::vec2(1.0f));
				alphaFromBlendingTexture(false);
				colorAccumulation(false);
				invisibleColor(glm::vec3(0.0f));
				invisibleColorThreshold(0.0f);
			}

			TexturedColorThreshold(const TexturedColorThreshold&) = delete;

			~TexturedColorThreshold()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
