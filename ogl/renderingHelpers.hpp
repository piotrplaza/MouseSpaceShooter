#pragma once

#include <functional>
#include <optional>

#include <GL/glew.h>

#include <glm/mat4x4.hpp>

#include <globals.hpp>
#include <components/texture.hpp>
#include <components/textureDef.hpp>
#include <components/mvp.hpp>

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
			{ (textureRatioPreserved ? (float)textureComponent.width / textureComponent.height : 1.0f)
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
			renderingTeardown = buffers.renderingSetup(shadersProgram.program);

		glBindVertexArray(buffers.vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, buffers.positionsCache.size());
		
		if (renderingTeardown)
			renderingTeardown();
	}
}
