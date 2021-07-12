#include "decorations.hpp"

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>
#include <ogl/renderingHelpers.hpp>
#include <ogl/oglHelpers.hpp>

#include <globals.hpp>

#include <components/decoration.hpp>
#include <components/mvp.hpp>
#include <components/graphicsSettings.hpp>
#include <components/screenInfo.hpp>
#include <components/lowResBuffers.hpp>

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
		Tools::UpdatePositionsBuffers(Globals::Components::farMidgroundDecorations, persistentBuffers.simpleFarMidgroundDecorations,
			persistentBuffers.texturedFarMidgroundDecorations, persistentBuffers.customShadersFarMidgroundDecorations);
		Tools::UpdatePositionsBuffers(Globals::Components::nearMidgroundDecorations, persistentBuffers.simpleNearMidgroundDecorations,
			persistentBuffers.texturedNearMidgroundDecorations, persistentBuffers.customShadersNearMidgroundDecorations);
		Tools::UpdatePositionsBuffers(Globals::Components::foregroundDecorations, persistentBuffers.simpleForegroundDecorations,
			persistentBuffers.texturedForegroundDecorations, persistentBuffers.customShadersForegroundDecorations);
	}

	void Decorations::updatePersistentTexCoordsBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components::backgroundDecorations, persistentBuffers.texturedBackgroundDecorations,
			persistentBuffers.customShadersBackgroundDecorations);
		Tools::UpdateTexCoordBuffers(Globals::Components::farMidgroundDecorations, persistentBuffers.texturedFarMidgroundDecorations,
			persistentBuffers.customShadersFarMidgroundDecorations);
		Tools::UpdateTexCoordBuffers(Globals::Components::nearMidgroundDecorations, persistentBuffers.texturedNearMidgroundDecorations,
			persistentBuffers.customShadersNearMidgroundDecorations);
		Tools::UpdateTexCoordBuffers(Globals::Components::foregroundDecorations, persistentBuffers.texturedForegroundDecorations,
			persistentBuffers.customShadersForegroundDecorations);
	}

	void Decorations::updateTemporaryPosAndTexCoordBuffers()
	{
		Tools::UpdatePosTexCoordBuffers(Globals::Components::temporaryBackgroundDecorations, temporaryBuffers.simpleBackgroundDecorations,
			temporaryBuffers.texturedBackgroundDecorations, temporaryBuffers.customShadersBackgroundDecorations);
		Tools::UpdatePosTexCoordBuffers(Globals::Components::temporaryFarMidgroundDecorations, temporaryBuffers.simpleFarMidgroundDecorations,
			temporaryBuffers.texturedFarMidgroundDecorations, temporaryBuffers.customShadersFarMidgroundDecorations);
		Tools::UpdatePosTexCoordBuffers(Globals::Components::temporaryNearMidgroundDecorations, temporaryBuffers.simpleNearMidgroundDecorations,
			temporaryBuffers.texturedNearMidgroundDecorations, temporaryBuffers.customShadersNearMidgroundDecorations);
		Tools::UpdatePosTexCoordBuffers(Globals::Components::temporaryForegroundDecorations, temporaryBuffers.simpleForegroundDecorations,
			temporaryBuffers.texturedForegroundDecorations, temporaryBuffers.customShadersForegroundDecorations);
	}

	void Decorations::step()
	{
		for (const auto* temporaryDecoration : { 
			&Globals::Components::temporaryBackgroundDecorations, 
			&Globals::Components::temporaryFarMidgroundDecorations,
			&Globals::Components::temporaryNearMidgroundDecorations,
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

	void Decorations::renderFarMidground() const
	{
		basicRender(persistentBuffers.simpleFarMidgroundDecorations, temporaryBuffers.simpleFarMidgroundDecorations);
		texturedRender(persistentBuffers.texturedFarMidgroundDecorations, temporaryBuffers.texturedFarMidgroundDecorations);
		customShadersRender(persistentBuffers.customShadersFarMidgroundDecorations, temporaryBuffers.customShadersFarMidgroundDecorations);
	}

	void Decorations::renderNearMidground() const
	{
		basicRender(persistentBuffers.simpleNearMidgroundDecorations, temporaryBuffers.simpleNearMidgroundDecorations);
		texturedRender(persistentBuffers.texturedNearMidgroundDecorations, temporaryBuffers.texturedNearMidgroundDecorations);
		customShadersRender(persistentBuffers.customShadersNearMidgroundDecorations, temporaryBuffers.customShadersNearMidgroundDecorations);
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
		bool anyLowRes = false;

		auto render = [&](const auto& buffers)
		{
			Tools::ConditionalScopedFramebuffer csfb(buffers.lowRes, Globals::Components::lowResBuffers.fbo,
				Globals::Components::lowResBuffers.size, Globals::Components::screenInfo.windowSize);

			if (!anyLowRes && buffers.lowRes)
			{
				anyLowRes = true;
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}

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

		if (anyLowRes)
			Tools::TexturedScreenRender(*texturedShadersProgram, Globals::Components::lowResBuffers.textureUnit - GL_TEXTURE0);
	}

	void Decorations::texturedRender(const std::vector<Buffers::PosTexCoordBuffers>& persistentBuffers,
		const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& temporaryBuffers) const
	{
		glUseProgram_proxy(texturedShadersProgram->getProgramId());
		texturedShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());

		bool anyLowRes = false;

		auto render = [&](const auto& buffers)
		{
			Tools::ConditionalScopedFramebuffer csfb(buffers.lowRes, Globals::Components::lowResBuffers.fbo,
				Globals::Components::lowResBuffers.size, Globals::Components::screenInfo.windowSize);

			if (!anyLowRes && buffers.lowRes)
			{
				anyLowRes = true;
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}

			texturedShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			texturedShadersProgram->modelUniform.setValue(glm::mat4(1.0f));

			Tools::TexturedRender(*texturedShadersProgram, buffers, *buffers.texture);
		};

		for (const auto& buffers : persistentBuffers)
			render(buffers);

		for (const auto& [id, buffers] : temporaryBuffers)
			render(buffers);

		if (anyLowRes)
			Tools::TexturedScreenRender(*texturedShadersProgram, Globals::Components::lowResBuffers.textureUnit - GL_TEXTURE0);
	}

	void Decorations::basicRender(const std::vector<Buffers::PosTexCoordBuffers>& persistentBuffers,
		const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& temporaryBuffers) const
	{
		glUseProgram_proxy(basicShadersProgram->getProgramId());
		basicShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());

		bool anyLowRes = false;

		auto render = [&](const auto& buffers)
		{
			Tools::ConditionalScopedFramebuffer csfb(buffers.lowRes, Globals::Components::lowResBuffers.fbo,
				Globals::Components::lowResBuffers.size, Globals::Components::screenInfo.windowSize);

			if (!anyLowRes && buffers.lowRes)
			{
				anyLowRes = true;
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}

			basicShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			basicShadersProgram->modelUniform.setValue(glm::mat4(1.0f));

			std::function<void()> renderingTeardown;
			if (buffers.renderingSetup)
				renderingTeardown = (*buffers.renderingSetup)(basicShadersProgram->getProgramId());

			glBindVertexArray(buffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, buffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		};

		for (const auto& buffers : persistentBuffers)
			render(buffers);

		for (const auto& [id, buffers] : temporaryBuffers)
			render(buffers);

		if (anyLowRes)
			Tools::TexturedScreenRender(*texturedShadersProgram, Globals::Components::lowResBuffers.textureUnit - GL_TEXTURE0);
	}
}
