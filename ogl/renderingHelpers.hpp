#include <functional>

#include <GL/glew.h>

#include <globals.hpp>

#include <components/texture.hpp>
#include <components/textureDef.hpp>

namespace Tools
{
	template <typename ShadersProgram, typename Buffers>
	void TexturedRender(ShadersProgram& shadersProgram, const Buffers& buffers, unsigned texture)
	{
		using namespace Globals::Components;

		const auto& textureComponent = textures[texture];
		const auto& textureDefComponent = texturesDef[texture];

		shadersProgram.texture1Uniform.setValue(texture);

		if (buffers.animationController)
		{
			const auto frameTransformation = buffers.animationController->getFrameTransformation();
			shadersProgram.textureTranslateUniform.setValue(frameTransformation.translate);
			shadersProgram.textureScaleUniform.setValue(frameTransformation.scale);
		}
		else
		{
			shadersProgram.textureTranslateUniform.setValue(textureDefComponent.translate);
			shadersProgram.textureScaleUniform.setValue(
				{ (buffers.textureRatioPreserved ? (float)textureComponent.width / textureComponent.height : 1.0f)
				* textureDefComponent.scale.x, textureDefComponent.scale.y });
		}

		std::function<void()> renderingTeardown;
		if (buffers.renderingSetup)
			renderingTeardown = buffers.renderingSetup(shadersProgram.program);

		glBindVertexArray(buffers.vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, buffers.positionsCache.size());
		
		if (renderingTeardown)
			renderingTeardown();
	}
}
