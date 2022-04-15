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
				mulBlendingColor(program, "mulBlendingColor"),
				addBlendingColor(program, "addBlendingColor"),
				numOfTextures(program, "numOfTextures"),
				textures(program, "textures"),
				texturesBaseTransform(program, "texturesBaseTransform"),
				texturesCustomTransform(program, "texturesCustomTransform"),
				alphaFromBlendingTexture(program, "alphaFromBlendingTexture"),
				colorAccumulation(program, "colorAccumulation"),
				textureCoordBasedOnModelTransform(program, "textureCoordBasedOnModelTransform"),
				numOfPlayers(program, "numOfPlayers"),
				playersCenter(program, "playersCenter"),
				playerUnhidingRadius(program, "playerUnhidingRadius")
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
			Uniforms::UniformController1b textureCoordBasedOnModelTransform;
			Uniforms::UniformController1i numOfPlayers;
			Uniforms::UniformController2fv<4> playersCenter;
			Uniforms::UniformController1f playerUnhidingRadius;
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
				mulBlendingColor(glm::vec4(1.0f));
				addBlendingColor(glm::vec4(0.0f));
				numOfTextures(1);
				texturesBaseTransform(glm::mat4(1.0f));
				texturesCustomTransform(glm::mat4(1.0f));
				alphaFromBlendingTexture(false);
				colorAccumulation(false);
				textureCoordBasedOnModelTransform(false);
				numOfPlayers(0);
				playersCenter(glm::vec2(0.0f, 0.0f));
				playerUnhidingRadius(0.0f);
			}

			SceneCoordTextured(const SceneCoordTextured&) = delete;

			~SceneCoordTextured()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
