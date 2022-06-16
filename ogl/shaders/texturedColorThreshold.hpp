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
				texturesBaseTransform(program, "texturesBaseTransform"),
				texturesCustomTransform(program, "texturesCustomTransform"),
				alphaFromBlendingTexture(program, "alphaFromBlendingTexture"),
				colorAccumulation(program, "colorAccumulation"),
				invisibleColor(program, "invisibleColor"),
				invisibleColorThreshold(program, "invisibleColorThreshold"),
				sceneCoordTextures(program, "sceneCoordTextures")
			{
			}

			Uniforms::UniformControllerMat4f model;
			Uniforms::UniformControllerMat4f vp;
			Uniforms::UniformController4f color;
			Uniforms::UniformController4f mulBlendingColor;
			Uniforms::UniformController4f addBlendingColor;
			Uniforms::UniformController1i numOfTextures;
			Uniforms::UniformController1iv<5> textures;
			Uniforms::UniformControllerMat4fv<5> texturesBaseTransform;
			Uniforms::UniformControllerMat4fv<5> texturesCustomTransform;
			Uniforms::UniformController1b alphaFromBlendingTexture;
			Uniforms::UniformController1b colorAccumulation;
			Uniforms::UniformController3f invisibleColor;
			Uniforms::UniformController1f invisibleColorThreshold;
			Uniforms::UniformController1b sceneCoordTextures;
		};

		struct TexturedColorThreshold: TexturedColorThresholdAccessor
		{
			TexturedColorThreshold():
				TexturedColorThresholdAccessor(LinkProgram(CompileShaders("ogl/shaders/texturedColorThreshold.vs",
					"ogl/shaders/texturedColorThreshold.fs"), { {0, "bPos"}, {1, "bColor"}, {2, "bTexCoord"} }))
			{
				model(glm::mat4(1.0f));
				vp(glm::mat4(1.0f));
				color(glm::vec4(1.0f));
				mulBlendingColor(glm::vec4(1.0f));
				addBlendingColor(glm::vec4(0.0f));
				numOfTextures(1);
				texturesBaseTransform(glm::mat4(1.0f));
				texturesCustomTransform(glm::mat4(1.0f));
				alphaFromBlendingTexture(false);
				colorAccumulation(false);
				invisibleColor(glm::vec3(0.0f));
				invisibleColorThreshold(0.0f);
				sceneCoordTextures(false);
			}

			TexturedColorThreshold(const TexturedColorThreshold&) = delete;

			~TexturedColorThreshold()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
