#include "standardRenderMode.hpp"

#include <ogl/shaders/textured.hpp>
#include <ogl/renderingHelpers.hpp>

#include <components/framebuffers.hpp>

TexturesFramebuffersRenderer::TexturesFramebuffersRenderer(ShadersUtils::Programs::Textured& texturedShadersProgram) :
	texturedShadersProgram(texturedShadersProgram)
{
}

TexturesFramebuffersRenderer::~TexturesFramebuffersRenderer()
{
	auto optionalRender = [&](const StandardRenderMode& renderMode) {
		if (renderMode.isMainMode())
			return;

		if (!ongoingModes[(size_t)renderMode.resolution][(size_t)renderMode.scaling][(size_t)renderMode.blending])
			return;

		if (renderMode.blending == StandardRenderMode::Blending::Standard)
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_ONE, GL_ONE);

		Tools::TexturedScreenRender(texturedShadersProgram, Globals::Components().defaultFramebuffers().getSubBuffers(renderMode).textureObject);

		if (renderMode.blending == StandardRenderMode::Blending::Additive)
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		};

		for (size_t res = 0; res < (size_t)StandardRenderMode::Resolution::COUNT; ++res)
			for (size_t scaling = 0; scaling < (size_t)StandardRenderMode::Scaling::COUNT; ++scaling)
				for (size_t blending = 0; blending < (size_t)StandardRenderMode::Blending::COUNT; ++blending)
					optionalRender({ (StandardRenderMode::Resolution)res, (StandardRenderMode::Scaling)scaling, (StandardRenderMode::Blending)blending });
}

void TexturesFramebuffersRenderer::clearIfFirstOfMode(const StandardRenderMode& renderMode)
{
	if (renderMode.isMainMode())
		return;

	if (auto& ongoing = ongoingModes[(size_t)renderMode.resolution][(size_t)renderMode.scaling][(size_t)renderMode.blending]; !ongoing)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ongoing = true;
	}
}

