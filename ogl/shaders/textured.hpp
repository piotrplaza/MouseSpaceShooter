#pragma once

#include "programBase.hpp"

namespace Shaders
{
	namespace Programs
	{
		struct TexturedAccessor : ProgramBase<TexturedAccessor>
		{
			using ProgramBase::ProgramBase;

			TexturedAccessor(ProgramId program):
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
				numOfPlayers(program, "numOfPlayers"),
				playersCenter(program, "playersCenter"),
				playerUnhidingRadius(program, "playerUnhidingRadius"),
				visibilityReduction(program, "visibilityReduction"),
				visibilityCenter(program, "visibilityCenter"),
				fullVisibilityDistance(program, "fullVisibilityDistance"),
				invisibilityDistance(program, "invisibilityDistance"),
				sceneCoordTextures(program, "sceneCoordTextures")
			{
			}

			Uniforms::UniformMat4f model;
			Uniforms::UniformMat4f vp;
			Uniforms::Uniform4f color;
			Uniforms::Uniform4f mulBlendingColor;
			Uniforms::Uniform4f addBlendingColor;
			Uniforms::Uniform1i numOfTextures;
			Uniforms::Uniform1iv<5> textures;
			Uniforms::UniformMat4fv<5> texturesBaseTransform;
			Uniforms::UniformMat4fv<5> texturesCustomTransform;
			Uniforms::Uniform1b alphaFromBlendingTexture;
			Uniforms::Uniform1b colorAccumulation;
			Uniforms::Uniform1i numOfPlayers;
			Uniforms::Uniform2fv<4> playersCenter;
			Uniforms::Uniform1f playerUnhidingRadius;
			Uniforms::Uniform1b visibilityReduction;
			Uniforms::Uniform2f visibilityCenter;
			Uniforms::Uniform1f fullVisibilityDistance;
			Uniforms::Uniform1f invisibilityDistance;
			Uniforms::Uniform1b sceneCoordTextures;
		};

		struct Textured : TexturedAccessor
		{
			Textured():
				TexturedAccessor(LinkProgram(CompileShaders("ogl/shaders/textured.vs",
					"ogl/shaders/textured.fs"), { {0, "bPos"}, {1, "bColor"}, {2, "bTexCoord"}, {3, "bNormal"} }))
			{
				model(glm::mat4(1.0f));
				vp(glm::mat4(1.0f));
				color(glm::vec4(1.0f));
				mulBlendingColor(glm::vec4(1.0f));
				addBlendingColor(glm::vec4(0.0f));
				numOfTextures(0);
				textures(0);
				texturesBaseTransform(glm::mat4(1.0f));
				texturesCustomTransform(glm::mat4(1.0f));
				alphaFromBlendingTexture(false);
				colorAccumulation(false);
				numOfPlayers(0);
				playersCenter(glm::vec2(0.0f, 0.0f));
				playerUnhidingRadius(0.0f);
				visibilityReduction(false);
				visibilityCenter({ 0.0f, 0.0f });
				fullVisibilityDistance(0.0f);
				invisibilityDistance(0.0f);
				sceneCoordTextures(false);
			}

			Textured(const Textured&) = delete;

			~Textured()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
