#include "decorations.hpp"

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>
#include <ogl/renderingHelpers.hpp>

#include <globals.hpp>

#include <components/decoration.hpp>
#include <components/mvp.hpp>

namespace Systems
{
	Decorations::Decorations()
	{
		initGraphics();
	}

	void Decorations::initGraphics()
	{
		basicShadersProgram = std::make_unique<Shaders::Programs::Basic>();
		texturedShadersProgram = std::make_unique<Shaders::Programs::Textured>();

		updatePositionsBuffers();
		updateTexCoordsBuffers();
	}

	void Decorations::updatePositionsBuffers()
	{
		Tools::UpdatePositionsBuffers(Globals::Components::backgroundDecorations, simpleBackgroundDecorationsBuffers,
			texturedBackgroundDecorationsBuffers, GL_STATIC_DRAW);
		Tools::UpdatePositionsBuffers(Globals::Components::midgroundDecorations, simpleMidgroundDecorationsBuffers,
			texturedMidgroundDecorationsBuffers, GL_STATIC_DRAW);
		Tools::UpdatePositionsBuffers(Globals::Components::foregroundDecorations, simpleForegroundDecorationsBuffers,
			texturedForegroundDecorationsBuffers, GL_STATIC_DRAW);
	}

	void Decorations::updateTexCoordsBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components::backgroundDecorations, texturedBackgroundDecorationsBuffers,
			GL_STATIC_DRAW);
		Tools::UpdateTexCoordBuffers(Globals::Components::midgroundDecorations, texturedMidgroundDecorationsBuffers,
			GL_STATIC_DRAW);
		Tools::UpdateTexCoordBuffers(Globals::Components::foregroundDecorations, texturedForegroundDecorationsBuffers,
			GL_STATIC_DRAW);
	}

	void Decorations::renderBackground() const
	{
		texturedRender(texturedBackgroundDecorationsBuffers);
		basicRender(simpleBackgroundDecorationsBuffers);
	}

	void Decorations::renderMidground() const
	{
		texturedRender(texturedMidgroundDecorationsBuffers);
		basicRender(simpleMidgroundDecorationsBuffers);
	}

	void Decorations::renderForeground() const
	{
		texturedRender(texturedForegroundDecorationsBuffers);
		basicRender(simpleForegroundDecorationsBuffers);
	}

	void Decorations::texturedRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const
	{
		glUseProgram_proxy(texturedShadersProgram->program);
		texturedShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());
		texturedShadersProgram->colorUniform.setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

		for (const auto& currentBuffers : buffers)
		{
			Tools::TexturedRender(*texturedShadersProgram, currentBuffers, *currentBuffers.texture);
		}
	}

	void Decorations::basicRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const
	{
		glUseProgram_proxy(basicShadersProgram->program);
		basicShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());
		basicShadersProgram->colorUniform.setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

		for (const auto& currentBuffers : buffers)
		{
			if (currentBuffers.renderingSetup) currentBuffers.renderingSetup(basicShadersProgram->program);
			glBindVertexArray(currentBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, currentBuffers.positionsCache.size());
		}
	}
}
