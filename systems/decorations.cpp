#include "decorations.hpp"

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>
#include <ogl/renderingHelpers.hpp>
#include <ogl/oglHelpers.hpp>

#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/textured.hpp>

#include <components/decoration.hpp>
#include <components/mvp.hpp>
#include <components/renderingSetup.hpp>
#include <components/graphicsSettings.hpp>
#include <components/screenInfo.hpp>
#include <components/framebuffers.hpp> 

#include <globals/shaders.hpp>

namespace Systems
{
	Decorations::Decorations() = default;

	void Decorations::postInit()
	{
		initGraphics();
	}

	void Decorations::step()
	{
		for (const auto* temporaryDecoration : { 
			&Globals::Components().temporaryBackgroundDecorations(),
			&Globals::Components().temporaryFarMidgroundDecorations(),
			&Globals::Components().temporaryNearMidgroundDecorations(),
			&Globals::Components().temporaryForegroundDecorations() })
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

	void Decorations::renderMidground() const
	{
		basicRender(persistentBuffers.simpleMidgroundDecorations, temporaryBuffers.simpleMidgroundDecorations);
		texturedRender(persistentBuffers.texturedMidgroundDecorations, temporaryBuffers.texturedMidgroundDecorations);
		customShadersRender(persistentBuffers.customShadersMidgroundDecorations, temporaryBuffers.customShadersMidgroundDecorations);
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

	void Decorations::initGraphics()
	{
		updatePersistentPositionsBuffers();
		updatePersistentTexCoordsBuffers();
	}

	void Decorations::updatePersistentPositionsBuffers()
	{
		Tools::UpdatePositionsBuffers(Globals::Components().backgroundDecorations(), persistentBuffers.simpleBackgroundDecorations,
			persistentBuffers.texturedBackgroundDecorations, persistentBuffers.customShadersBackgroundDecorations);
		Tools::UpdatePositionsBuffers(Globals::Components().farMidgroundDecorations(), persistentBuffers.simpleFarMidgroundDecorations,
			persistentBuffers.texturedFarMidgroundDecorations, persistentBuffers.customShadersFarMidgroundDecorations);
		Tools::UpdatePositionsBuffers(Globals::Components().midgroundDecorations(), persistentBuffers.simpleMidgroundDecorations,
			persistentBuffers.texturedMidgroundDecorations, persistentBuffers.customShadersMidgroundDecorations);
		Tools::UpdatePositionsBuffers(Globals::Components().nearMidgroundDecorations(), persistentBuffers.simpleNearMidgroundDecorations,
			persistentBuffers.texturedNearMidgroundDecorations, persistentBuffers.customShadersNearMidgroundDecorations);
		Tools::UpdatePositionsBuffers(Globals::Components().foregroundDecorations(), persistentBuffers.simpleForegroundDecorations,
			persistentBuffers.texturedForegroundDecorations, persistentBuffers.customShadersForegroundDecorations);
	}

	void Decorations::updatePersistentTexCoordsBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components().backgroundDecorations(), persistentBuffers.texturedBackgroundDecorations,
			persistentBuffers.customShadersBackgroundDecorations);
		Tools::UpdateTexCoordBuffers(Globals::Components().farMidgroundDecorations(), persistentBuffers.texturedFarMidgroundDecorations,
			persistentBuffers.customShadersFarMidgroundDecorations);
		Tools::UpdateTexCoordBuffers(Globals::Components().midgroundDecorations(), persistentBuffers.texturedMidgroundDecorations,
			persistentBuffers.customShadersMidgroundDecorations);
		Tools::UpdateTexCoordBuffers(Globals::Components().nearMidgroundDecorations(), persistentBuffers.texturedNearMidgroundDecorations,
			persistentBuffers.customShadersNearMidgroundDecorations);
		Tools::UpdateTexCoordBuffers(Globals::Components().foregroundDecorations(), persistentBuffers.texturedForegroundDecorations,
			persistentBuffers.customShadersForegroundDecorations);
	}

	void Decorations::updateTemporaryPosAndTexCoordBuffers()
	{
		Tools::UpdatePosTexCoordBuffers(Globals::Components().temporaryBackgroundDecorations(), temporaryBuffers.simpleBackgroundDecorations,
			temporaryBuffers.texturedBackgroundDecorations, temporaryBuffers.customShadersBackgroundDecorations);
		Tools::UpdatePosTexCoordBuffers(Globals::Components().temporaryFarMidgroundDecorations(), temporaryBuffers.simpleFarMidgroundDecorations,
			temporaryBuffers.texturedFarMidgroundDecorations, temporaryBuffers.customShadersFarMidgroundDecorations);
		Tools::UpdatePosTexCoordBuffers(Globals::Components().temporaryMidgroundDecorations(), temporaryBuffers.simpleMidgroundDecorations,
			temporaryBuffers.texturedMidgroundDecorations, temporaryBuffers.customShadersMidgroundDecorations);
		Tools::UpdatePosTexCoordBuffers(Globals::Components().temporaryNearMidgroundDecorations(), temporaryBuffers.simpleNearMidgroundDecorations,
			temporaryBuffers.texturedNearMidgroundDecorations, temporaryBuffers.customShadersNearMidgroundDecorations);
		Tools::UpdatePosTexCoordBuffers(Globals::Components().temporaryForegroundDecorations(), temporaryBuffers.simpleForegroundDecorations,
			temporaryBuffers.texturedForegroundDecorations, temporaryBuffers.customShadersForegroundDecorations);
	}

	void Decorations::customShadersRender(const std::vector<Buffers::GenericBuffers>& persistentBuffers,
		const std::unordered_map<ComponentId, Buffers::GenericBuffers>& temporaryBuffers) const
	{
		TexturesFramebuffersRenderer texturesFramebuffersRenderer(Globals::Shaders().textured());

		auto render = [&](const auto& buffers)
		{
			const auto& lowResSubBuffers = Globals::Components().framebuffers().getSubBuffers(buffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(buffers.resolutionMode != ResolutionMode::Normal, lowResSubBuffers.fbo,
				lowResSubBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

			texturesFramebuffersRenderer.clearIfFirstOfMode(buffers.resolutionMode);

			assert(buffers.customShadersProgram);
			glUseProgram_proxy(*buffers.customShadersProgram);

			std::function<void()> renderingTeardown;
			if (buffers.renderingSetup)
				renderingTeardown = Globals::Components().renderingSetups()[buffers.renderingSetup](*buffers.customShadersProgram);

			buffers.draw();

			if (renderingTeardown)
				renderingTeardown();
		};

		for (const auto& buffers : persistentBuffers)
			render(buffers);

		for (const auto& [id, buffers] : temporaryBuffers)
			render(buffers);
	}

	void Decorations::texturedRender(const std::vector<Buffers::GenericBuffers>& persistentBuffers,
		const std::unordered_map<ComponentId, Buffers::GenericBuffers>& temporaryBuffers) const
	{
		glUseProgram_proxy(Globals::Shaders().textured().getProgramId());
		Globals::Shaders().textured().vp(Globals::Components().mvp().getVP());

		TexturesFramebuffersRenderer texturesFramebuffersRenderer(Globals::Shaders().textured());

		auto render = [&](const auto& buffers)
		{
			const auto& lowResSubBuffers = Globals::Components().framebuffers().getSubBuffers(buffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(buffers.resolutionMode != ResolutionMode::Normal, lowResSubBuffers.fbo,
				lowResSubBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

			texturesFramebuffersRenderer.clearIfFirstOfMode(buffers.resolutionMode);

			Globals::Shaders().textured().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().textured().model(glm::mat4(1.0f));

			Tools::TexturedRender(Globals::Shaders().textured(), buffers, buffers.texture);
		};

		for (const auto& buffers : persistentBuffers)
			render(buffers);

		for (const auto& [id, buffers] : temporaryBuffers)
			render(buffers);
	}

	void Decorations::basicRender(const std::vector<Buffers::GenericBuffers>& persistentBuffers,
		const std::unordered_map<ComponentId, Buffers::GenericBuffers>& temporaryBuffers) const
	{
		glUseProgram_proxy(Globals::Shaders().basic().getProgramId());
		Globals::Shaders().basic().vp(Globals::Components().mvp().getVP());

		TexturesFramebuffersRenderer texturesFramebuffersRenderer(Globals::Shaders().textured());

		auto render = [&](const auto& buffers)
		{
			const auto& lowResSubBuffers = Globals::Components().framebuffers().getSubBuffers(buffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(buffers.resolutionMode != ResolutionMode::Normal, lowResSubBuffers.fbo,
				lowResSubBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

			texturesFramebuffersRenderer.clearIfFirstOfMode(buffers.resolutionMode);

			Globals::Shaders().basic().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().basic().model(glm::mat4(1.0f));

			std::function<void()> renderingTeardown;
			if (buffers.renderingSetup)
				renderingTeardown = Globals::Components().renderingSetups()[buffers.renderingSetup](Globals::Shaders().basic().getProgramId());

			buffers.draw();

			if (renderingTeardown)
				renderingTeardown();
		};

		for (const auto& buffers : persistentBuffers)
			render(buffers);

		for (const auto& [id, buffers] : temporaryBuffers)
			render(buffers);
	}
}
