#pragma once

#include "programBase.hpp"

namespace Shaders
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
				fogAmplification(program, "fogAmplification")
			{
			}

			Uniforms::UniformMat4f model;
			Uniforms::UniformMat4f vp;
			Uniforms::UniformMat3f normalMatrix;
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
			Uniforms::Uniform3f playerCenter;
			Uniforms::Uniform1f playerUnhidingRadius;
			Uniforms::Uniform1b visibilityReduction;
			Uniforms::Uniform3f visibilityCenter;
			Uniforms::Uniform1f fullVisibilityDistance;
			Uniforms::Uniform1f invisibilityDistance;
			Uniforms::Uniform1b sceneCoordTextures;
			Uniforms::Uniform3f clearColor;
			Uniforms::Uniform1i numOfLights;
			Uniforms::Uniform3fv<128> lightsPos;
			Uniforms::Uniform3fv<128> lightsCol;
			Uniforms::Uniform1fv<128> lightsAttenuation;
			Uniforms::Uniform1fv<128> lightsDarkColorFactor;
			Uniforms::Uniform1f ambient;
			Uniforms::Uniform1f diffuse;
			Uniforms::Uniform3f viewPos;
			Uniforms::Uniform1f specular;
			Uniforms::Uniform1f specularFocus;
			Uniforms::Uniform1f specularMaterialColorFactor;
			Uniforms::Uniform4f illumination;
			Uniforms::Uniform3f darkColor;
			Uniforms::Uniform1b flatColor;
			Uniforms::Uniform1b flatNormal;
			Uniforms::Uniform1b lightModelColorNormalization;
			Uniforms::Uniform1b lightModelEnabled;
			Uniforms::Uniform1f alphaDiscardTreshold;
			Uniforms::Uniform1b gpuSideInstancedNormalTransforms;
			Uniforms::Uniform1f fogAmplification;
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
			}

			TexturedPhong(const Textured&) = delete;

			~TexturedPhong()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
