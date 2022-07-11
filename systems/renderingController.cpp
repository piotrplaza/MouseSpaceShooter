#include "renderingController.hpp"

#include <components/screenInfo.hpp>
#include <components/graphicsSettings.hpp>
#include <components/framebuffers.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/renderingBuffers.hpp>

#include <systems/structures.hpp>
#include <systems/actors.hpp>
#include <systems/temporaries.hpp>
#include <systems/decorations.hpp>

#include <ogl/oglHelpers.hpp>
#include <ogl/renderingHelpers.hpp>

#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/textured.hpp>

#include <globals/shaders.hpp>
#include <globals/systems.hpp>

namespace Systems
{
	RenderingController::RenderingController()
	{
		Globals::Components().mainFramebufferRenderer().renderer = Tools::StandardFullscreenRenderer(Globals::Shaders().textured());
	}

	void RenderingController::postInit() const
	{
	}

	void RenderingController::render() const
	{
		const auto& screenInfo = Globals::Components().screenInfo();
		const auto& framebuffers = Globals::Components().framebuffers();
		const glm::vec4& clearColor = Globals::Components().graphicsSettings().clearColor;

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.main.fbo);
		glViewport(0, 0, framebuffers.main.size.x, framebuffers.main.size.y);
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (size_t layer = 0; layer < (size_t)RenderLayer::COUNT; ++layer)
		{
			basicRender(layer);
			texturedRender(layer);
			customShadersRender(layer);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenInfo.windowSize.x, screenInfo.windowSize.y);
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		assert(Globals::Components().mainFramebufferRenderer().renderer);
		Globals::Components().mainFramebufferRenderer().renderer(framebuffers.main.textureUnit - GL_TEXTURE0);
	}

	void RenderingController::basicRender(size_t layer) const
	{
		const auto& staticBuffers = Globals::Components().renderingBuffers().staticBuffers.basic;
		const auto& dynamicBuffers = Globals::Components().renderingBuffers().dynamicBuffers.basic;

		glUseProgram_proxy(Globals::Shaders().basic().getProgramId());
		Globals::Shaders().basic().vp(Globals::Components().mvp().getVP());
		Globals::Shaders().basic().color(Globals::Components().graphicsSettings().defaultColor);

		TexturesFramebuffersRenderer texturesFramebuffersRenderer(Globals::Shaders().textured());

		auto render = [&](const auto& buffers)
		{
			const auto& lowResSubBuffers = Globals::Components().framebuffers().getSubBuffers(*buffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(*buffers.resolutionMode != ResolutionMode::Normal, lowResSubBuffers.fbo,
				lowResSubBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

			texturesFramebuffersRenderer.clearIfFirstOfMode(*buffers.resolutionMode);

			buffers.draw(Globals::Shaders().basic().getProgramId(), [](const auto& buffers) {
				Globals::Shaders().basic().model(buffers.modelMatrixF ? buffers.modelMatrixF() : glm::mat4(1.0f));
				});
		};

		for (const auto& buffers : staticBuffers[layer])
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers[layer])
			render(buffers);
	}

	void RenderingController::texturedRender(size_t layer) const
	{
		const auto& staticBuffers = Globals::Components().renderingBuffers().staticBuffers.textured;
		const auto& dynamicBuffers = Globals::Components().renderingBuffers().dynamicBuffers.textured;

		glUseProgram_proxy(Globals::Shaders().textured().getProgramId());
		Globals::Shaders().textured().vp(Globals::Components().mvp().getVP());
		Globals::Shaders().textured().color(Globals::Components().graphicsSettings().defaultColor);

		TexturesFramebuffersRenderer texturesFramebuffersRenderer(Globals::Shaders().textured());

		auto render = [&](const auto& buffers)
		{
			const auto& lowResSubBuffers = Globals::Components().framebuffers().getSubBuffers(*buffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(*buffers.resolutionMode != ResolutionMode::Normal, lowResSubBuffers.fbo,
				lowResSubBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

			texturesFramebuffersRenderer.clearIfFirstOfMode(*buffers.resolutionMode);

			buffers.draw(Globals::Shaders().textured(), [](const auto& buffers) {
				Globals::Shaders().textured().model(buffers.modelMatrixF ? buffers.modelMatrixF() : glm::mat4(1.0f));
				Tools::PrepareTexturedRender(Globals::Shaders().textured(), buffers, *buffers.texture);
				});
		};

		for (const auto& buffers : staticBuffers[layer])
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers[layer])
			render(buffers);
	}

	void RenderingController::customShadersRender(size_t layer) const
	{
		const auto& staticBuffers = Globals::Components().renderingBuffers().staticBuffers.customShaders;
		const auto& dynamicBuffers = Globals::Components().renderingBuffers().dynamicBuffers.customShaders;

		TexturesFramebuffersRenderer texturesFramebuffersRenderer(Globals::Shaders().textured());

		auto render = [&](const auto& buffers)
		{
			const auto& lowResSubBuffers = Globals::Components().framebuffers().getSubBuffers(*buffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(*buffers.resolutionMode != ResolutionMode::Normal, lowResSubBuffers.fbo,
				lowResSubBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

			texturesFramebuffersRenderer.clearIfFirstOfMode(*buffers.resolutionMode);

			assert(buffers.customShadersProgram);
			glUseProgram_proxy(**buffers.customShadersProgram);

			buffers.draw(**buffers.customShadersProgram, [](auto&) {});
		};

		for (const auto& buffers : staticBuffers[layer])
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers[layer])
			render(buffers);
	}
}
