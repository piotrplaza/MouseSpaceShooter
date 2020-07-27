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
			texturedBackgroundDecorationsBuffers, customShadersBackgroundDecorationsBuffers, GL_STATIC_DRAW);
		Tools::UpdatePositionsBuffers(Globals::Components::midgroundDecorations, simpleMidgroundDecorationsBuffers,
			texturedMidgroundDecorationsBuffers, customShadersMidgroundDecorationsBuffers, GL_STATIC_DRAW);
		Tools::UpdatePositionsBuffers(Globals::Components::foregroundDecorations, simpleForegroundDecorationsBuffers,
			texturedForegroundDecorationsBuffers, customShadersForegroundDecorationsBuffers, GL_STATIC_DRAW);
	}

	void Decorations::updateTexCoordsBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components::backgroundDecorations, texturedBackgroundDecorationsBuffers,
			customShadersBackgroundDecorationsBuffers, GL_STATIC_DRAW);
		Tools::UpdateTexCoordBuffers(Globals::Components::midgroundDecorations, texturedMidgroundDecorationsBuffers,
			customShadersMidgroundDecorationsBuffers, GL_STATIC_DRAW);
		Tools::UpdateTexCoordBuffers(Globals::Components::foregroundDecorations, texturedForegroundDecorationsBuffers,
			customShadersForegroundDecorationsBuffers, GL_STATIC_DRAW);
	}

	void Decorations::renderBackground() const
	{
		customShadersRender(customShadersBackgroundDecorationsBuffers);
		texturedRender(texturedBackgroundDecorationsBuffers);
		basicRender(simpleBackgroundDecorationsBuffers);
	}

	void Decorations::renderMidground() const
	{
		customShadersRender(customShadersMidgroundDecorationsBuffers);
		texturedRender(texturedMidgroundDecorationsBuffers);
		basicRender(simpleMidgroundDecorationsBuffers);
	}

	void Decorations::renderForeground() const
	{
		customShadersRender(customShadersForegroundDecorationsBuffers);
		texturedRender(texturedForegroundDecorationsBuffers);
		basicRender(simpleForegroundDecorationsBuffers);
	}

	void Decorations::customShadersRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const
	{
		for (const auto& currentBuffers : buffers)
		{
			glUseProgram_proxy(*currentBuffers.customShadersProgram);

			std::function<void()> renderingTeardown;
			if (currentBuffers.renderingSetup)
				renderingTeardown = currentBuffers.renderingSetup(*currentBuffers.customShadersProgram);

			glBindVertexArray(currentBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, currentBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}
	}

	void Decorations::texturedRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const
	{
		glUseProgram_proxy(texturedShadersProgram->program);
		texturedShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());

		for (const auto& currentBuffers : buffers)
		{
			texturedShadersProgram->colorUniform.setValue({ 1.0f, 1.0f, 1.0f, 1.0f });
			texturedShadersProgram->modelUniform.setValue(glm::mat4(1.0f));

			Tools::TexturedRender(*texturedShadersProgram, currentBuffers, *currentBuffers.texture);
		}
	}

	void Decorations::basicRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const
	{
		glUseProgram_proxy(basicShadersProgram->program);
		basicShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());

		for (const auto& currentBuffers : buffers)
		{
			basicShadersProgram->colorUniform.setValue({ 1.0f, 1.0f, 1.0f, 1.0f });
			basicShadersProgram->modelUniform.setValue(glm::mat4(1.0f));

			std::function<void()> renderingTeardown;
			if (currentBuffers.renderingSetup)
				renderingTeardown = currentBuffers.renderingSetup(basicShadersProgram->program);

			glBindVertexArray(currentBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, currentBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}
	}
}
