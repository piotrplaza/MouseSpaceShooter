#include "resolutionMode.hpp"

#include <ogl/shaders/textured.hpp>
#include <ogl/renderingHelpers.hpp>

#include <components/framebuffers.hpp>

TexturesFramebuffersRenderer::TexturesFramebuffersRenderer(ShadersUtils::Programs::Textured& texturedShadersProgram):
	texturedShadersProgram(texturedShadersProgram)
{
}

TexturesFramebuffersRenderer::~TexturesFramebuffersRenderer()
{
	auto optionalRender = [&](const ResolutionMode& mode) {
		if (mode.isMainMode())
			return;

		if (!ongoingModes[(size_t)mode.resolution][(size_t)mode.scaling][(size_t)mode.blending])
			return;

		if (mode.blending == ResolutionMode::Blending::Standard)
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_ONE, GL_ONE);

		Tools::TexturedScreenRender(texturedShadersProgram, Globals::Components().framebuffers().getSubBuffers(mode).textureUnit - GL_TEXTURE0);

		if (mode.blending == ResolutionMode::Blending::Additive)
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	};

	for (size_t res = 0; res < (size_t)ResolutionMode::Resolution::COUNT; ++res)
		for (size_t scaling = 0; scaling < (size_t)ResolutionMode::Scaling::COUNT; ++scaling)
			for (size_t blending = 0; blending < (size_t)ResolutionMode::Blending::COUNT; ++blending)
				optionalRender({ (ResolutionMode::Resolution)res, (ResolutionMode::Scaling)scaling, (ResolutionMode::Blending)blending });
}

void TexturesFramebuffersRenderer::clearIfFirstOfMode(ResolutionMode mode)
{
	if (mode.isMainMode())
		return;

	if (auto& ongoing = ongoingModes[(size_t)mode.resolution][(size_t)mode.scaling][(size_t)mode.blending]; !ongoing)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ongoing = true;
	}
}
