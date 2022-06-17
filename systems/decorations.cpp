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
		for (const auto* decoration : {
			&Globals::Components().backgroundDecorations(),
			&Globals::Components().farMidgroundDecorations(),
			&Globals::Components().nearMidgroundDecorations(),
			&Globals::Components().foregroundDecorations() })
		{
			for (const auto& decoration : *decoration)
				if (decoration.step)
					decoration.step();
		}

		for (const auto* dynamicDecoration : { 
			&Globals::Components().dynamicBackgroundDecorations(),
			&Globals::Components().dynamicFarMidgroundDecorations(),
			&Globals::Components().dynamicNearMidgroundDecorations(),
			&Globals::Components().dynamicForegroundDecorations() })
		{
			for (const auto& [id, decoration] : *dynamicDecoration)
				if (decoration.step)
					decoration.step();
		}

		updateDynamicBuffers();
	}

	void Decorations::renderBackground() const
	{
		basicRender(staticBuffers.simpleBackgroundDecorations, dynamicBuffers.simpleBackgroundDecorations);
		texturedRender(staticBuffers.texturedBackgroundDecorations, dynamicBuffers.texturedBackgroundDecorations);
		customShadersRender(staticBuffers.customShadersBackgroundDecorations, dynamicBuffers.customShadersBackgroundDecorations);
	}

	void Decorations::renderFarMidground() const
	{
		basicRender(staticBuffers.simpleFarMidgroundDecorations, dynamicBuffers.simpleFarMidgroundDecorations);
		texturedRender(staticBuffers.texturedFarMidgroundDecorations, dynamicBuffers.texturedFarMidgroundDecorations);
		customShadersRender(staticBuffers.customShadersFarMidgroundDecorations, dynamicBuffers.customShadersFarMidgroundDecorations);
	}

	void Decorations::renderMidground() const
	{
		basicRender(staticBuffers.simpleMidgroundDecorations, dynamicBuffers.simpleMidgroundDecorations);
		texturedRender(staticBuffers.texturedMidgroundDecorations, dynamicBuffers.texturedMidgroundDecorations);
		customShadersRender(staticBuffers.customShadersMidgroundDecorations, dynamicBuffers.customShadersMidgroundDecorations);
	}

	void Decorations::renderNearMidground() const
	{
		basicRender(staticBuffers.simpleNearMidgroundDecorations, dynamicBuffers.simpleNearMidgroundDecorations);
		texturedRender(staticBuffers.texturedNearMidgroundDecorations, dynamicBuffers.texturedNearMidgroundDecorations);
		customShadersRender(staticBuffers.customShadersNearMidgroundDecorations, dynamicBuffers.customShadersNearMidgroundDecorations);
	}

	void Decorations::renderForeground() const
	{
		basicRender(staticBuffers.simpleForegroundDecorations, dynamicBuffers.simpleForegroundDecorations);
		texturedRender(staticBuffers.texturedForegroundDecorations, dynamicBuffers.texturedForegroundDecorations);
		customShadersRender(staticBuffers.customShadersForegroundDecorations, dynamicBuffers.customShadersForegroundDecorations);
	}

	void Decorations::initGraphics()
	{
		updateStaticBuffers();
	}

	void Decorations::updateStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().backgroundDecorations(), staticBuffers.simpleBackgroundDecorations,
			staticBuffers.texturedBackgroundDecorations, staticBuffers.customShadersBackgroundDecorations);
		Tools::UpdateStaticBuffers(Globals::Components().farMidgroundDecorations(), staticBuffers.simpleFarMidgroundDecorations,
			staticBuffers.texturedFarMidgroundDecorations, staticBuffers.customShadersFarMidgroundDecorations);
		Tools::UpdateStaticBuffers(Globals::Components().midgroundDecorations(), staticBuffers.simpleMidgroundDecorations,
			staticBuffers.texturedMidgroundDecorations, staticBuffers.customShadersMidgroundDecorations);
		Tools::UpdateStaticBuffers(Globals::Components().nearMidgroundDecorations(), staticBuffers.simpleNearMidgroundDecorations,
			staticBuffers.texturedNearMidgroundDecorations, staticBuffers.customShadersNearMidgroundDecorations);
		Tools::UpdateStaticBuffers(Globals::Components().foregroundDecorations(), staticBuffers.simpleForegroundDecorations,
			staticBuffers.texturedForegroundDecorations, staticBuffers.customShadersForegroundDecorations);
	}

	void Decorations::updateDynamicBuffers()
	{
		Tools::UpdateDynamicBuffers(Globals::Components().dynamicBackgroundDecorations(), dynamicBuffers.simpleBackgroundDecorations,
			dynamicBuffers.texturedBackgroundDecorations, dynamicBuffers.customShadersBackgroundDecorations);
		Tools::UpdateDynamicBuffers(Globals::Components().dynamicFarMidgroundDecorations(), dynamicBuffers.simpleFarMidgroundDecorations,
			dynamicBuffers.texturedFarMidgroundDecorations, dynamicBuffers.customShadersFarMidgroundDecorations);
		Tools::UpdateDynamicBuffers(Globals::Components().dynamicMidgroundDecorations(), dynamicBuffers.simpleMidgroundDecorations,
			dynamicBuffers.texturedMidgroundDecorations, dynamicBuffers.customShadersMidgroundDecorations);
		Tools::UpdateDynamicBuffers(Globals::Components().dynamicNearMidgroundDecorations(), dynamicBuffers.simpleNearMidgroundDecorations,
			dynamicBuffers.texturedNearMidgroundDecorations, dynamicBuffers.customShadersNearMidgroundDecorations);
		Tools::UpdateDynamicBuffers(Globals::Components().dynamicForegroundDecorations(), dynamicBuffers.simpleForegroundDecorations,
			dynamicBuffers.texturedForegroundDecorations, dynamicBuffers.customShadersForegroundDecorations);
	}

	void Decorations::customShadersRender(const std::vector<Buffers::GenericBuffers>& staticBuffers,
		const std::unordered_map<ComponentId, Buffers::GenericBuffers>& dynamicBuffers) const
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

		for (const auto& buffers : staticBuffers)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers)
			render(buffers);
	}

	void Decorations::texturedRender(const std::vector<Buffers::GenericBuffers>& staticBuffers,
		const std::unordered_map<ComponentId, Buffers::GenericBuffers>& dynamicBuffers) const
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

		for (const auto& buffers : staticBuffers)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers)
			render(buffers);
	}

	void Decorations::basicRender(const std::vector<Buffers::GenericBuffers>& staticBuffers,
		const std::unordered_map<ComponentId, Buffers::GenericBuffers>& dynamicBuffers) const
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

		for (const auto& buffers : staticBuffers)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers)
			render(buffers);
	}
}
