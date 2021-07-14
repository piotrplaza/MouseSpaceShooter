#include "resolutionMode.hpp"

#include <ogl/shaders/textured.hpp>
#include <ogl/renderingHelpers.hpp>

#include <globals.hpp>
#include <components/lowResBuffers.hpp>

TexturesFramebuffersRenderer::TexturesFramebuffersRenderer(Shaders::Programs::Textured& texturedShadersProgram):
	texturedShadersProgram(texturedShadersProgram)
{
}

TexturesFramebuffersRenderer::~TexturesFramebuffersRenderer()
{
	if (lowerLinear)
	{
		glBlendFunc(GL_ONE, GL_ONE);
		Tools::TexturedScreenRender(texturedShadersProgram, Globals::Components::lowResBuffers.lowerLinear.textureUnit - GL_TEXTURE0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	if (lowestLinear)
	{
		glBlendFunc(GL_ONE, GL_ONE);
		Tools::TexturedScreenRender(texturedShadersProgram, Globals::Components::lowResBuffers.lowestLinear.textureUnit - GL_TEXTURE0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	if (pixelArt)
		Tools::TexturedScreenRender(texturedShadersProgram, Globals::Components::lowResBuffers.pixelArt.textureUnit - GL_TEXTURE0);
}

void TexturesFramebuffersRenderer::clearIfFirstOfMode(ResolutionMode resolutionMode)
{
	auto clrbuf = []()
	{
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	};

	switch (resolutionMode)
	{
	case ResolutionMode::LowerLinear:
		if (!lowerLinear)
		{
			lowerLinear = true;
			clrbuf();
		}
		return;

	case ResolutionMode::LowestLinear:
		if (!lowestLinear)
		{
			lowestLinear = true;
			clrbuf();
		}
		return;

	case ResolutionMode::PixelArt:
		if (!pixelArt)
		{
			pixelArt = true;
			clrbuf();
		}
		return;
	}
}
