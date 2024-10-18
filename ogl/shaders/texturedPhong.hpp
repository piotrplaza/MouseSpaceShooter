#pragma once

#include "programBase.hpp"

namespace ShadersUtils
{
	namespace Programs
	{
		struct TexturedPhongAccessor : ProgramBase<TexturedPhongAccessor>
		{
			using ProgramBase::ProgramBase;

			TexturedPhongAccessor(ProgramId program) :
				ProgramBase(program),
				model(program, "model"),
				vp(program, "vp"),
				normalMatrix(program, "normalMatrix"),
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
				playerCenter(program, "playerCenter"),
				playerUnhidingRadius(program, "playerUnhidingRadius"),
				visibilityReduction(program, "visibilityReduction"),
				visibilityCenter(program, "visibilityCenter"),
				fullVisibilityDistance(program, "fullVisibilityDistance"),
				invisibilityDistance(program, "invisibilityDistance"),
				sceneCoordTextures(program, "sceneCoordTextures"),
				clearColor(program, "clearColor"),
				numOfLights(program, "numOfLights"),
				lightsPos(program, "lightsPos"),
				lightsCol(program, "lightsCol"),
				lightsAttenuation(program, "lightsAttenuation"),
				lightsDarkColorFactor(program, "lightsDarkColorFactor"),
				ambient(program, "ambient"),
				diffuse(program, "diffuse"),
				viewPos(program, "viewPos"),
				specular(program, "specular"),
				specularFocus(program, "specularFocus"),
				specularMaterialColorFactor(program, "specularMaterialColorFactor"),
				illumination(program, "illumination"),
				darkColor(program, "darkColor"),
				flatColor(program, "flatColor"),
				flatNormal(program, "flatNormal"),
				lightModelColorNormalization(program, "lightModelColorNormalization"),
				lightModelEnabled(program, "lightModelEnabled"),
				alphaDiscardTreshold(program, "alphaDiscardTreshold"),
				gpuSideInstancedNormalTransforms(program, "gpuSideInstancedNormalTransforms"),
				fogAmplification(program, "fogAmplification"),
				forcedAlpha(program, "forcedAlpha")
			{
			}

			UniformsUtils::UniformMat4f model;
			UniformsUtils::UniformMat4f vp;
			UniformsUtils::UniformMat3f normalMatrix;
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
			UniformsUtils::Uniform3f playerCenter;
			UniformsUtils::Uniform1f playerUnhidingRadius;
			UniformsUtils::Uniform1b visibilityReduction;
			UniformsUtils::Uniform3f visibilityCenter;
			UniformsUtils::Uniform1f fullVisibilityDistance;
			UniformsUtils::Uniform1f invisibilityDistance;
			UniformsUtils::Uniform1b sceneCoordTextures;
			UniformsUtils::Uniform3f clearColor;
			UniformsUtils::Uniform1i numOfLights;
			UniformsUtils::Uniform3fv<128> lightsPos;
			UniformsUtils::Uniform3fv<128> lightsCol;
			UniformsUtils::Uniform1fv<128> lightsAttenuation;
			UniformsUtils::Uniform1fv<128> lightsDarkColorFactor;
			UniformsUtils::Uniform1f ambient;
			UniformsUtils::Uniform1f diffuse;
			UniformsUtils::Uniform3f viewPos;
			UniformsUtils::Uniform1f specular;
			UniformsUtils::Uniform1f specularFocus;
			UniformsUtils::Uniform1f specularMaterialColorFactor;
			UniformsUtils::Uniform4f illumination;
			UniformsUtils::Uniform3f darkColor;
			UniformsUtils::Uniform1b flatColor;
			UniformsUtils::Uniform1b flatNormal;
			UniformsUtils::Uniform1b lightModelColorNormalization;
			UniformsUtils::Uniform1b lightModelEnabled;
			UniformsUtils::Uniform1f alphaDiscardTreshold;
			UniformsUtils::Uniform1b gpuSideInstancedNormalTransforms;
			UniformsUtils::Uniform1f fogAmplification;
			UniformsUtils::Uniform1f forcedAlpha;
		};

		struct TexturedPhong : TexturedPhongAccessor
		{
			TexturedPhong() :
				TexturedPhongAccessor(LinkProgram(CompileShaders("ogl/shaders/texturedPhong.vs",
					"ogl/shaders/texturedPhong.fs"), { {0, "bPos"}, {1, "bColor"}, {2, "bTexCoord"}, {3, "bNormal"}, {4, "bInstancedTransform"}, {8, "bInstancedNormalTransform"} }))
			{
				model(glm::mat4(1.0f));
				vp(glm::mat4(1.0f));
				normalMatrix(glm::mat3(1.0f));
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
				playerCenter(glm::vec3(0.0f));
				playerUnhidingRadius(0.0f);
				visibilityReduction(false);
				visibilityCenter(glm::vec3(0.0f));
				fullVisibilityDistance(0.0f);
				invisibilityDistance(0.0f);
				sceneCoordTextures(false);
				clearColor(glm::vec3(0.0f));
				numOfLights(0);
				lightsPos(glm::vec3(0.0f));
				lightsCol(glm::vec3(1.0f));
				lightsAttenuation(0.0f);
				lightsDarkColorFactor(0.0f);
				ambient(1.0f);
				diffuse(0.0f);
				viewPos(glm::vec3(0.0f));
				specular(0.0f);
				specularFocus(16.0f);
				specularMaterialColorFactor(0.0f);
				illumination(glm::vec4(0.0f));
				darkColor(glm::vec3(0.0f));
				flatColor(false);
				flatNormal(false);
				lightModelColorNormalization(false);
				lightModelEnabled(true);
				alphaDiscardTreshold(0.5f);
				gpuSideInstancedNormalTransforms(false);
				fogAmplification(0.0f);
				forcedAlpha(-1.0f);
			}

			TexturedPhong(const Textured&) = delete;

			~TexturedPhong()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
