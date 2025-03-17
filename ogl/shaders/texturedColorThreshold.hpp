#pragma once

#include "programBase.hpp"

namespace ShadersUtils
{
	namespace Programs
	{
		struct TexturedColorThresholdAccessor : ProgramBaseCRTP<TexturedColorThresholdAccessor>
		{
			using ProgramBaseCRTP::ProgramBaseCRTP;

			TexturedColorThresholdAccessor(ProgramId program):
				ProgramBaseCRTP(program),
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

			UniformsUtils::UniformMat4f model;
			UniformsUtils::UniformMat4f vp;
			UniformsUtils::Uniform4f color;
			UniformsUtils::Uniform4f mulBlendingColor;
			UniformsUtils::Uniform4f addBlendingColor;
			UniformsUtils::Uniform1i numOfTextures;
			UniformsUtils::Uniform1iv<5> textures;
			UniformsUtils::UniformMat4fv<5> texturesBaseTransform;
			UniformsUtils::UniformMat4fv<5> texturesCustomTransform;
			UniformsUtils::Uniform1b alphaFromBlendingTexture;
			UniformsUtils::Uniform1b colorAccumulation;
			UniformsUtils::Uniform3f invisibleColor;
			UniformsUtils::Uniform1f invisibleColorThreshold;
			UniformsUtils::Uniform1b sceneCoordTextures;
		};

		struct TexturedColorThreshold : TexturedColorThresholdAccessor
		{
			TexturedColorThreshold():
				TexturedColorThresholdAccessor(LinkProgram(CompileShaders("ogl/shaders/texturedColorThreshold.vs",
					"ogl/shaders/texturedColorThreshold.fs"), { {0, "bPos"}, {1, "bColor"}, {2, "bTexCoord"}, {4, "bInstancedTransform"} }))
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
