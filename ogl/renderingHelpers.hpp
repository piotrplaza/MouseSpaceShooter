#pragma once

#include <functional>
#include <optional>

#include <GL/glew.h>

#include <glm/mat4x4.hpp>

#include <globals.hpp>
#include <components/texture.hpp>
#include <components/textureDef.hpp>
#include <components/mvp.hpp>

#include <ogl/oglProxy.hpp>

namespace Tools
{
	template <typename ShadersProgram>
	inline void MVPInitialization(ShadersProgram& shadersProgram, std::optional<glm::mat4> modelMatrix = std::nullopt)
	{
		shadersProgram.vpUniform.setValue(Globals::Components::mvp.getVP());
		shadersProgram.modelUniform.setValue(modelMatrix ? *modelMatrix : glm::mat4(1.0f));
	}

	template <typename ShadersProgram>
	inline void StaticTexturedRenderInitialization(ShadersProgram& shadersProgram, unsigned texture, bool textureRatioPreserved)
	{
		const auto& textureComponent = Globals::Components::textures[texture];
		const auto& textureDefComponent = Globals::Components::texturesDef[texture];

		shadersProgram.texture1Uniform.setValue(texture);
		shadersProgram.textureTranslateUniform.setValue(textureDefComponent.translate);
		shadersProgram.textureScaleUniform.setValue(
			{ (textureRatioPreserved ? (float)textureComponent.size.x / textureComponent.size.y : 1.0f)
			* textureDefComponent.scale.x, textureDefComponent.scale.y });
	}

	template <typename ShadersProgram, typename AnimationController>
	inline void AnimatedTexturedRenderInitialization(ShadersProgram& shadersProgram, unsigned texture,
		const AnimationController& animationController)
	{
		const auto& textureComponent = Globals::Components::textures[texture];
		const auto& textureDefComponent = Globals::Components::texturesDef[texture];

		shadersProgram.texture1Uniform.setValue(texture);
		const auto frameTransformation = animationController.getFrameTransformation();
		shadersProgram.textureTranslateUniform.setValue(frameTransformation.translate);
		shadersProgram.textureScaleUniform.setValue(frameTransformation.scale);

	}

	template <typename ShadersProgram, typename Buffers>
	inline void TexturedRender(ShadersProgram& shadersProgram, const Buffers& buffers, unsigned texture)
	{
		if (buffers.animationController)
			AnimatedTexturedRenderInitialization(shadersProgram, texture, *buffers.animationController);
		else
			StaticTexturedRenderInitialization(shadersProgram, texture, buffers.textureRatioPreserved);

		std::function<void()> renderingTeardown;
		if (buffers.renderingSetup)
			renderingTeardown = (*buffers.renderingSetup)(shadersProgram.getProgramId());

		glBindVertexArray(buffers.vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, buffers.positionsCache.size());
		
		if (renderingTeardown)
			renderingTeardown();
	}

	template <typename ShadersPrograms>
	inline void TexturedScreenRender(ShadersPrograms& shadersProgram, unsigned texture)
	{
		const int numOfVertices = 6;

		const static glm::vec3 positions[numOfVertices] =
		{
			{-1.0f, -1.0f, 0.0f},
			{1.0f, -1.0f, 0.0f},
			{-1.0f, 1.0f, 0.0f},
			{-1.0f, 1.0f, 0.0f},
			{1.0f, -1.0f, 0.0f},
			{1.0f, 1.0f, 0.0f}
		};

		const static glm::vec2 texCoords[numOfVertices] =
		{
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{0.0f, 1.0f},
			{0.0f, 1.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f}
		};

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, &positions);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, &texCoords);
		glEnableVertexAttribArray(1);

		glUseProgram_proxy(shadersProgram.getProgramId());

		shadersProgram.modelUniform.setValue(glm::mat4(1.0f));
		shadersProgram.vpUniform.setValue(glm::mat4(1.0f));
		shadersProgram.colorUniform.setValue(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		shadersProgram.textureTranslateUniform.setValue(glm::vec2(0.0f));
		shadersProgram.textureScaleUniform.setValue(glm::vec2(1.0f));
		shadersProgram.texture1Uniform.setValue(texture);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDrawArrays(GL_TRIANGLES, 0, numOfVertices);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}
