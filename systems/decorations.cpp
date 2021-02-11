#include "decorations.hpp"

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>
#include <ogl/renderingHelpers.hpp>

#include <globals.hpp>

#include <components/decoration.hpp>
#include <components/mvp.hpp>
#include <components/graphicsSettings.hpp>

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

		updatePersistentPositionsBuffers();
		updatePersistentTexCoordsBuffers();
	}

	void Decorations::updatePersistentPositionsBuffers()
	{
		Tools::UpdatePositionsBuffers(Globals::Components::backgroundDecorations, persistentBuffers.simpleBackgroundDecorations,
			persistentBuffers.texturedBackgroundDecorations, persistentBuffers.customShadersBackgroundDecorations);
		Tools::UpdatePositionsBuffers(Globals::Components::midgroundDecorations, persistentBuffers.simpleMidgroundDecorations,
			persistentBuffers.texturedMidgroundDecorations, persistentBuffers.customShadersMidgroundDecorations);
		Tools::UpdatePositionsBuffers(Globals::Components::foregroundDecorations, persistentBuffers.simpleForegroundDecorations,
			persistentBuffers.texturedForegroundDecorations, persistentBuffers.customShadersForegroundDecorations);
	}

	void Decorations::updatePersistentTexCoordsBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components::backgroundDecorations, persistentBuffers.texturedBackgroundDecorations,
			persistentBuffers.customShadersBackgroundDecorations);
		Tools::UpdateTexCoordBuffers(Globals::Components::midgroundDecorations, persistentBuffers.texturedMidgroundDecorations,
			persistentBuffers.customShadersMidgroundDecorations);
		Tools::UpdateTexCoordBuffers(Globals::Components::foregroundDecorations, persistentBuffers.texturedForegroundDecorations,
			persistentBuffers.customShadersForegroundDecorations);
	}

	void Decorations::updateTemporaryPosAndTexCoordBuffers()
	{
		Tools::UpdatePosTexCoordBuffers(Globals::Components::temporaryBackgroundDecorations, temporaryBuffers.simpleBackgroundDecorations,
			temporaryBuffers.texturedBackgroundDecorations, temporaryBuffers.customShadersBackgroundDecorations);
		Tools::UpdatePosTexCoordBuffers(Globals::Components::temporaryMidgroundDecorations, temporaryBuffers.simpleMidgroundDecorations,
			temporaryBuffers.texturedMidgroundDecorations, temporaryBuffers.customShadersMidgroundDecorations);
		Tools::UpdatePosTexCoordBuffers(Globals::Components::temporaryForegroundDecorations, temporaryBuffers.simpleForegroundDecorations,
			temporaryBuffers.texturedForegroundDecorations, temporaryBuffers.customShadersForegroundDecorations);
	}

	void Decorations::step()
	{
		for (const auto* temporaryDecoration : { 
			&Globals::Components::temporaryBackgroundDecorations, 
			&Globals::Components::temporaryMidgroundDecorations, 
			&Globals::Components::temporaryForegroundDecorations })
		{
			for (const auto& [id, decoration] : *temporaryDecoration)
				if (decoration.step)
					decoration.step();
		}

		updateTemporaryPosAndTexCoordBuffers();
	}

	void Decorations::renderBackground() const
	{
		basicRender(persistentBuffers.simpleBackgroundDecorations, temporaryBuffers.simpleBackgroundDecorations);
		texturedRender(persistentBuffers.texturedBackgroundDecorations, temporaryBuffers.texturedBackgroundDecorations);
		customShadersRender(persistentBuffers.customShadersBackgroundDecorations, temporaryBuffers.customShadersBackgroundDecorations);
	}

	void Decorations::renderMidground() const
	{
		basicRender(persistentBuffers.simpleMidgroundDecorations, temporaryBuffers.simpleMidgroundDecorations);
		texturedRender(persistentBuffers.texturedMidgroundDecorations, temporaryBuffers.texturedMidgroundDecorations);
		customShadersRender(persistentBuffers.customShadersMidgroundDecorations, temporaryBuffers.customShadersMidgroundDecorations);
	}

	void Decorations::renderForeground() const
	{
		basicRender(persistentBuffers.simpleForegroundDecorations, temporaryBuffers.simpleForegroundDecorations);
		texturedRender(persistentBuffers.texturedForegroundDecorations, temporaryBuffers.texturedForegroundDecorations);
		customShadersRender(persistentBuffers.customShadersForegroundDecorations, temporaryBuffers.customShadersForegroundDecorations);
	}

	void Decorations::customShadersRender(const std::vector<Buffers::PosTexCoordBuffers>& persistentBuffers,
		const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& temporaryBuffers) const
	{
		auto render = [](const auto& buffers)
		{
			assert(buffers.customShadersProgram);
			glUseProgram_proxy(*buffers.customShadersProgram);

			std::function<void()> renderingTeardown;
			if (buffers.renderingSetup)
				renderingTeardown = (*buffers.renderingSetup)(*buffers.customShadersProgram);

			glBindVertexArray(buffers.vertexArray);
			glDrawArrays(buffers.drawMode, 0, buffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		};

		for (const auto& buffers : persistentBuffers)
			render(buffers);

		for (const auto& [id, buffers] : temporaryBuffers)
			render(buffers);
	}

	void Decorations::texturedRender(const std::vector<Buffers::PosTexCoordBuffers>& persistentBuffers,
		const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& temporaryBuffers) const
	{
		glUseProgram_proxy(texturedShadersProgram->program);
		texturedShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());

		auto render = [&](const auto& buffers)
		{
			texturedShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			texturedShadersProgram->modelUniform.setValue(glm::mat4(1.0f));
			Tools::TexturedRender(*texturedShadersProgram, buffers, *buffers.texture);
		};

		for (const auto& buffers : persistentBuffers)
			render(buffers);

		for (const auto& [id, buffers] : temporaryBuffers)
			render(buffers);
	}

	void Decorations::basicRender(const std::vector<Buffers::PosTexCoordBuffers>& persistentBuffers,
		const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& temporaryBuffers) const
	{
		glUseProgram_proxy(basicShadersProgram->program);
		basicShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());

		auto render = [&](const auto& buffers)
		{
			basicShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			basicShadersProgram->modelUniform.setValue(glm::mat4(1.0f));

			std::function<void()> renderingTeardown;
			if (buffers.renderingSetup)
				renderingTeardown = (*buffers.renderingSetup)(basicShadersProgram->program);

			glBindVertexArray(buffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, buffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		};

		for (const auto& buffers : persistentBuffers)
			render(buffers);

		for (const auto& [id, buffers] : temporaryBuffers)
			render(buffers);
	}
}
