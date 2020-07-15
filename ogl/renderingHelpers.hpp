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
		shadersProgram.textureTranslateUniform.setValue(textureDefComponent.translate);
		shadersProgram.textureScaleUniform.setValue(
			{ (float)textureComponent.width / textureComponent.height * textureDefComponent.scale.x, textureDefComponent.scale.y });
		if (buffers.renderingSetup) buffers.renderingSetup(shadersProgram.program);
		glBindVertexArray(buffers.vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, buffers.positionsCache.size());
	}
}
