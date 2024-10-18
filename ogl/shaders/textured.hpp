#pragma once

#include "programBase.hpp"

namespace ShadersUtils
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
				sceneCoordTextures(program, "sceneCoordTextures"),
				forcedAlpha(program, "forcedAlpha")
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
			UniformsUtils::Uniform1i numOfPlayers;
			UniformsUtils::Uniform2fv<4> playersCenter;
			UniformsUtils::Uniform1f playerUnhidingRadius;
			UniformsUtils::Uniform1b visibilityReduction;
			UniformsUtils::Uniform2f visibilityCenter;
			UniformsUtils::Uniform1f fullVisibilityDistance;
			UniformsUtils::Uniform1f invisibilityDistance;
			UniformsUtils::Uniform1b sceneCoordTextures;
			UniformsUtils::Uniform1f forcedAlpha;
		};

		struct Textured : TexturedAccessor
		{
			Textured():
				TexturedAccessor(LinkProgram(CompileShaders("ogl/shaders/textured.vs",
					"ogl/shaders/textured.fs"), { {0, "bPos"}, {1, "bColor"}, {2, "bTexCoord"}, {3, "bNormal"}, {4, "bInstancedTransform"} }))
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
				forcedAlpha(-1.0f);
			}

			Textured(const Textured&) = delete;

			~Textured()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
