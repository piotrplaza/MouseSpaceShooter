#pragma once

#include "programBase.hpp"

namespace Shaders
{
	namespace Programs
	{
		struct SceneCoordTexturedAccessor : ProgramBase
		{
			using ProgramBase::ProgramBase;

			SceneCoordTexturedAccessor(ProgramId program):
				ProgramBase(program),
				model(program, "model"),
				vp(program, "vp"),
				color(program, "color"),
				blendingColor(program, "blendingColor"),
				numOfTextures(program, "numOfTextures"),
				textures(program, "textures"),
				texturesTranslate(program, "texturesTranslate"),
				texturesScale(program, "texturesScale"),
				alphaFromBlendingTexture(program, "alphaFromBlendingTexture"),
				colorAccumulation(program, "colorAccumulation"),
				textureCoordBasedOnModelTransform(program, "textureCoordBasedOnModelTransform")
			{
			}

			Uniforms::UniformControllerMat4f model;
			Uniforms::UniformControllerMat4f vp;
			Uniforms::UniformController4f color;
			Uniforms::UniformController4f blendingColor;
			Uniforms::UniformController1i numOfTextures;
			Uniforms::UniformController1iv<5> textures;
			Uniforms::UniformController2fv<5> texturesTranslate;
			Uniforms::UniformController2fv<5> texturesScale;
			Uniforms::UniformController1b alphaFromBlendingTexture;
			Uniforms::UniformController1b colorAccumulation;
			Uniforms::UniformController1b textureCoordBasedOnModelTransform;
		};

		struct SceneCoordTextured: SceneCoordTexturedAccessor
		{
			SceneCoordTextured():
				SceneCoordTexturedAccessor(LinkProgram(CompileShaders("ogl/shaders/sceneCoordTextured.vs",
					"ogl/shaders/sceneCoordTextured.fs"), { {0, "bPos"} }))
			{
				model(glm::mat4(1.0f));
				vp(glm::mat4(1.0f));
				color(glm::vec4(1.0f));
				blendingColor(glm::vec4(1.0f));
				numOfTextures(1);
				texturesTranslate(glm::vec2(0.0f));
				texturesScale(glm::vec2(1.0f));
				alphaFromBlendingTexture(false);
				colorAccumulation(false);
				textureCoordBasedOnModelTransform(false);
			}

			SceneCoordTextured(const SceneCoordTextured&) = delete;

			~SceneCoordTextured()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
