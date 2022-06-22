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
			&Globals::Components().midgroundDecorations(),
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
			&Globals::Components().dynamicMidgroundDecorations(),
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

	void Decorations::updateBackgroundStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().backgroundDecorations(), staticBuffers.simpleBackgroundDecorations,
			staticBuffers.texturedBackgroundDecorations, staticBuffers.customShadersBackgroundDecorations);
	}

	void Decorations::updateFarMidgroundStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().farMidgroundDecorations(), staticBuffers.simpleFarMidgroundDecorations,
			staticBuffers.texturedFarMidgroundDecorations, staticBuffers.customShadersFarMidgroundDecorations);
	}

	void Decorations::updateMidgroundStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().midgroundDecorations(), staticBuffers.simpleMidgroundDecorations,
			staticBuffers.texturedMidgroundDecorations, staticBuffers.customShadersMidgroundDecorations);
	}

	void Decorations::updateNearMidgroundStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().nearMidgroundDecorations(), staticBuffers.simpleNearMidgroundDecorations,
			staticBuffers.texturedNearMidgroundDecorations, staticBuffers.customShadersNearMidgroundDecorations);
	}

	void Decorations::updateForegroundStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().foregroundDecorations(), staticBuffers.simpleForegroundDecorations,
			staticBuffers.texturedForegroundDecorations, staticBuffers.customShadersForegroundDecorations);
	}

	void Decorations::updateStaticBuffers()
	{
		updateBackgroundStaticBuffers();
		updateFarMidgroundStaticBuffers();
		updateMidgroundStaticBuffers();
		updateNearMidgroundStaticBuffers();
		updateForegroundStaticBuffers();
	}

	void Decorations::initGraphics()
	{
		updateStaticBuffers();
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

			buffers.draw(*buffers.customShadersProgram, [](auto&) {});
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
		Globals::Shaders().textured().color(Globals::Components().graphicsSettings().defaultColor);

		TexturesFramebuffersRenderer texturesFramebuffersRenderer(Globals::Shaders().textured());

		auto render = [&](const auto& buffers)
		{
			const auto& lowResSubBuffers = Globals::Components().framebuffers().getSubBuffers(buffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(buffers.resolutionMode != ResolutionMode::Normal, lowResSubBuffers.fbo,
				lowResSubBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

			texturesFramebuffersRenderer.clearIfFirstOfMode(buffers.resolutionMode);

			buffers.draw(Globals::Shaders().textured(), [](const auto& buffers) {
				Globals::Shaders().textured().model(buffers.modelMatrixF ? buffers.modelMatrixF() : glm::mat4(1.0f));
				Tools::PrepareTexturedRender(Globals::Shaders().textured(), buffers, buffers.texture);
				});
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
		Globals::Shaders().basic().color(Globals::Components().graphicsSettings().defaultColor);

		TexturesFramebuffersRenderer texturesFramebuffersRenderer(Globals::Shaders().textured());

		auto render = [&](const auto& buffers)
		{
			const auto& lowResSubBuffers = Globals::Components().framebuffers().getSubBuffers(buffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(buffers.resolutionMode != ResolutionMode::Normal, lowResSubBuffers.fbo,
				lowResSubBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

			texturesFramebuffersRenderer.clearIfFirstOfMode(buffers.resolutionMode);

			buffers.draw(Globals::Shaders().basic().getProgramId(), [](const auto& buffers) {
				Globals::Shaders().basic().model(buffers.modelMatrixF ? buffers.modelMatrixF() : glm::mat4(1.0f));
				});
		};

		for (const auto& buffers : staticBuffers)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers)
			render(buffers);
	}
}
