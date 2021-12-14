#include "resolutionMode.hpp"

#include <ogl/shaders/textured.hpp>
#include <ogl/renderingHelpers.hpp>

#include <globals.hpp>
#include <components/framebuffers.hpp>

TexturesFramebuffersRenderer::TexturesFramebuffersRenderer(Shaders::Programs::Textured& texturedShadersProgram):
	texturedShadersProgram(texturedShadersProgram)
{
}

TexturesFramebuffersRenderer::~TexturesFramebuffersRenderer()
{
	if (lowPixelArtBlend0)
	{
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		Tools::TexturedScreenRender(texturedShadersProgram, Globals::Components().framebuffers().lowPixelArtBlend0.textureUnit - GL_TEXTURE0);
	}

	if (pixelArtBlend0)
	{
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		Tools::TexturedScreenRender(texturedShadersProgram, Globals::Components().framebuffers().pixelArtBlend0.textureUnit - GL_TEXTURE0);
	}

	if (lowestLinearBlend0)
	{
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		Tools::TexturedScreenRender(texturedShadersProgram, Globals::Components().framebuffers().lowestLinearBlend0.textureUnit - GL_TEXTURE0);
	}

	if (lowerLinearBlend0)
	{
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		Tools::TexturedScreenRender(texturedShadersProgram, Globals::Components().framebuffers().lowerLinearBlend0.textureUnit - GL_TEXTURE0);
	}

	if (lowPixelArtBlend1)
	{
		glBlendFunc(GL_ONE, GL_ONE);
		Tools::TexturedScreenRender(texturedShadersProgram, Globals::Components().framebuffers().lowPixelArtBlend1.textureUnit - GL_TEXTURE0);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	if (pixelArtBlend1)
	{
		glBlendFunc(GL_ONE, GL_ONE);
		Tools::TexturedScreenRender(texturedShadersProgram, Globals::Components().framebuffers().pixelArtBlend1.textureUnit - GL_TEXTURE0);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	if (lowestLinearBlend1)
	{
		glBlendFunc(GL_ONE, GL_ONE);
		Tools::TexturedScreenRender(texturedShadersProgram, Globals::Components().framebuffers().lowestLinearBlend1.textureUnit - GL_TEXTURE0);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	if (lowerLinearBlend1)
	{
		glBlendFunc(GL_ONE, GL_ONE);
		Tools::TexturedScreenRender(texturedShadersProgram, Globals::Components().framebuffers().lowerLinearBlend1.textureUnit - GL_TEXTURE0);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}
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
	case ResolutionMode::LowerLinearBlend0:
		if (!lowerLinearBlend0)
		{
			lowerLinearBlend0 = true;
			clrbuf();
		}
		return;

	case ResolutionMode::LowerLinearBlend1:
		if (!lowerLinearBlend1)
		{
			lowerLinearBlend1 = true;
			clrbuf();
		}
		return;

	case ResolutionMode::LowestLinearBlend0:
		if (!lowestLinearBlend0)
		{
			lowestLinearBlend0 = true;
			clrbuf();
		}
		return;

	case ResolutionMode::LowestLinearBlend1:
		if (!lowestLinearBlend1)
		{
			lowestLinearBlend1 = true;
			clrbuf();
		}
		return;

	case ResolutionMode::PixelArtBlend0:
		if (!pixelArtBlend0)
		{
			pixelArtBlend0 = true;
			clrbuf();
		}
		return;

	case ResolutionMode::PixelArtBlend1:
		if (!pixelArtBlend1)
		{
			pixelArtBlend1 = true;
			clrbuf();
		}
		return;

	case ResolutionMode::LowPixelArtBlend0:
		if (!lowPixelArtBlend0)
		{
			lowPixelArtBlend0 = true;
			clrbuf();
		}
		return;

	case ResolutionMode::LowPixelArtBlend1:
		if (!lowPixelArtBlend1)
		{
			lowPixelArtBlend1 = true;
			clrbuf();
		}
		return;
	}
}
