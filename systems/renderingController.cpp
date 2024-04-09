#include "renderingController.hpp"

#include <components/screenInfo.hpp>
#include <components/graphicsSettings.hpp>
#include <components/framebuffers.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/renderingBuffers.hpp>
#include <components/mvp.hpp>

#include <systems/structures.hpp>
#include <systems/actors.hpp>
#include <systems/temporaries.hpp>
#include <systems/decorations.hpp>

#include <ogl/oglHelpers.hpp>
#include <ogl/renderingHelpers.hpp>

#include <ogl/shaders/basicPhong.hpp>
#include <ogl/shaders/texturedPhong.hpp>
#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/textured.hpp>

#include <globals/shaders.hpp>
#include <globals/systems.hpp>

namespace
{
	void BasicPhongRender(size_t layer, TexturesFramebuffersRenderer& texturesFramebuffersRenderer)
	{
		const auto& staticBuffers = Globals::Components().renderingBuffers().staticBuffers.basicPhong;
		const auto& dynamicBuffers = Globals::Components().renderingBuffers().dynamicBuffers.basicPhong;

		glUseProgram_proxy(Globals::Shaders().basicPhong().getProgramId());
		Globals::Shaders().basicPhong().vp(Globals::Components().mvp3D().getVP());

		auto render = [&](const auto& buffers) {
			const auto& subBuffers = Globals::Components().framebuffers().getSubBuffers(*buffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(*buffers.resolutionMode != ResolutionMode::Normal, subBuffers.fbo,
				subBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

			texturesFramebuffersRenderer.clearIfFirstOfMode(*buffers.resolutionMode);

			buffers.draw(Globals::Shaders().basicPhong().getProgramId(), [](const auto& buffers) {
				const auto modelMatrix = (buffers.renderable->modelMatrixF)();
				Globals::Shaders().basicPhong().model(modelMatrix);
				Globals::Shaders().basicPhong().normalMatrix(Globals::Components().mvp3D().getNormalMatrix(modelMatrix));
				Globals::Shaders().basicPhong().color((buffers.renderable->colorF) ? (buffers.renderable->colorF)() : Globals::Components().graphicsSettings().defaultColor);
				Globals::Shaders().basicPhong().ambient(buffers.renderable->params3D->ambient_);
				Globals::Shaders().basicPhong().diffuse(buffers.renderable->params3D->diffuse_);
				Globals::Shaders().basicPhong().specular(buffers.renderable->params3D->specular_);
				Globals::Shaders().basicPhong().specularFocus(buffers.renderable->params3D->specularFocus_);
				Globals::Shaders().basicPhong().specularMaterialColorFactor(buffers.renderable->params3D->specularMaterialColorFactor_);
				Globals::Shaders().basicPhong().illumination((buffers.renderable->params3D->illuminationF_) ? (buffers.renderable->params3D->illuminationF_)() : glm::vec4(0.0f));
				Globals::Shaders().basicPhong().lightModelEnabled(buffers.renderable->params3D->lightModelEnabled_);
				Globals::Shaders().basicPhong().gpuSideInstancedNormalTransforms(buffers.renderable->params3D->gpuSideInstancedNormalTransforms_);
			});
		};

		for (const auto& buffers : staticBuffers[layer])
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers[layer])
			render(buffers);
	}

	void TexturedPhongRender(size_t layer, TexturesFramebuffersRenderer& texturesFramebuffersRenderer)
	{
		const auto& staticBuffers = Globals::Components().renderingBuffers().staticBuffers.texturedPhong;
		const auto& dynamicBuffers = Globals::Components().renderingBuffers().dynamicBuffers.texturedPhong;

		glUseProgram_proxy(Globals::Shaders().texturedPhong().getProgramId());
		Globals::Shaders().texturedPhong().vp(Globals::Components().mvp3D().getVP());

		auto render = [&](const auto& buffers) {
			const auto& subBuffers = Globals::Components().framebuffers().getSubBuffers(*buffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(*buffers.resolutionMode != ResolutionMode::Normal, subBuffers.fbo,
				subBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

			texturesFramebuffersRenderer.clearIfFirstOfMode(*buffers.resolutionMode);

			buffers.draw(Globals::Shaders().texturedPhong(), [](const auto& buffers) {
				const auto modelMatrix = (buffers.renderable->modelMatrixF)();
				Globals::Shaders().texturedPhong().model(modelMatrix);
				Globals::Shaders().texturedPhong().normalMatrix(Globals::Components().mvp3D().getNormalMatrix(modelMatrix));
				Globals::Shaders().texturedPhong().color((buffers.renderable->colorF) ? (buffers.renderable->colorF)() : Globals::Components().graphicsSettings().defaultColor);
				Globals::Shaders().texturedPhong().ambient(buffers.renderable->params3D->ambient_);
				Globals::Shaders().texturedPhong().diffuse(buffers.renderable->params3D->diffuse_);
				Globals::Shaders().texturedPhong().specular(buffers.renderable->params3D->specular_);
				Globals::Shaders().texturedPhong().specularFocus(buffers.renderable->params3D->specularFocus_);
				Globals::Shaders().texturedPhong().specularMaterialColorFactor(buffers.renderable->params3D->specularMaterialColorFactor_);
				Globals::Shaders().texturedPhong().illumination((buffers.renderable->params3D->illuminationF_) ? (buffers.renderable->params3D->illuminationF_)() : glm::vec4(0.0f));
				Globals::Shaders().texturedPhong().lightModelEnabled(buffers.renderable->params3D->lightModelEnabled_);
				Globals::Shaders().texturedPhong().alphaDiscardTreshold(buffers.renderable->params3D->alphaDiscardTreshold_);
				Globals::Shaders().texturedPhong().gpuSideInstancedNormalTransforms(buffers.renderable->params3D->gpuSideInstancedNormalTransforms_);
				Tools::PrepareTexturedRender(Globals::Shaders().texturedPhong(), buffers.renderable->texture, buffers.renderable->preserveTextureRatio);
			});
		};

		for (const auto& buffers : staticBuffers[layer])
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers[layer])
			render(buffers);
	}

	void BasicRender(size_t layer, TexturesFramebuffersRenderer& texturesFramebuffersRenderer)
	{
		const auto& staticBuffers = Globals::Components().renderingBuffers().staticBuffers.basic;
		const auto& dynamicBuffers = Globals::Components().renderingBuffers().dynamicBuffers.basic;

		glUseProgram_proxy(Globals::Shaders().basic().getProgramId());
		Globals::Shaders().basic().vp(Globals::Components().mvp2D().getVP());

		auto render = [&](const auto& buffers) {
			const auto& subBuffers = Globals::Components().framebuffers().getSubBuffers(*buffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(*buffers.resolutionMode != ResolutionMode::Normal, subBuffers.fbo,
				subBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

			texturesFramebuffersRenderer.clearIfFirstOfMode(*buffers.resolutionMode);

			buffers.draw(Globals::Shaders().basic().getProgramId(), [](const auto& buffers) {
				Globals::Shaders().basic().model((buffers.renderable->modelMatrixF)());
				Globals::Shaders().basic().color((buffers.renderable->colorF) ? (buffers.renderable->colorF)() : Globals::Components().graphicsSettings().defaultColor);
			});
		};

		for (const auto& buffers : staticBuffers[layer])
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers[layer])
			render(buffers);
	}

	void TexturedRender(size_t layer, TexturesFramebuffersRenderer& texturesFramebuffersRenderer)
	{
		const auto& staticBuffers = Globals::Components().renderingBuffers().staticBuffers.textured;
		const auto& dynamicBuffers = Globals::Components().renderingBuffers().dynamicBuffers.textured;

		glUseProgram_proxy(Globals::Shaders().textured().getProgramId());
		Globals::Shaders().textured().vp(Globals::Components().mvp2D().getVP());

		auto render = [&](const auto& buffers) {
			const auto& subBuffers = Globals::Components().framebuffers().getSubBuffers(*buffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(*buffers.resolutionMode != ResolutionMode::Normal, subBuffers.fbo,
				subBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

			texturesFramebuffersRenderer.clearIfFirstOfMode(*buffers.resolutionMode);

			buffers.draw(Globals::Shaders().textured(), [](const auto& buffers) {
				Globals::Shaders().textured().model((buffers.renderable->modelMatrixF)());
				Globals::Shaders().textured().visibilityCenter((buffers.renderable->originF)());
				Globals::Shaders().textured().color((buffers.renderable->colorF) ? (buffers.renderable->colorF)() : Globals::Components().graphicsSettings().defaultColor);
				Tools::PrepareTexturedRender(Globals::Shaders().textured(), buffers.renderable->texture, buffers.renderable->preserveTextureRatio);
			});
		};

		for (const auto& buffers : staticBuffers[layer])
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers[layer])
			render(buffers);
	}

	void CustomShadersRender(size_t layer, TexturesFramebuffersRenderer& texturesFramebuffersRenderer)
	{
		const auto& staticBuffers = Globals::Components().renderingBuffers().staticBuffers.customShaders;
		const auto& dynamicBuffers = Globals::Components().renderingBuffers().dynamicBuffers.customShaders;

		auto render = [&](const auto& buffers) {
			const auto& subBuffers = Globals::Components().framebuffers().getSubBuffers(*buffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(*buffers.resolutionMode != ResolutionMode::Normal, subBuffers.fbo,
				subBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

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

		Tools::Lights3DSetup(Globals::Shaders().basicPhong());
		Tools::Lights3DSetup(Globals::Shaders().texturedPhong());

		const auto viewPos = Globals::Components().mvp3D().getViewPos();
		Globals::Shaders().basicPhong().viewPos(viewPos);
		Globals::Shaders().texturedPhong().viewPos(viewPos);

		Globals::Shaders().frameSetup();

		for (size_t layer = 0; layer < (size_t)RenderLayer::COUNT; ++layer)
		{
			TexturesFramebuffersRenderer texturesFramebuffersRenderer(Globals::Shaders().textured());

			glEnable(GL_DEPTH_TEST);
			BasicPhongRender(layer, texturesFramebuffersRenderer);
			TexturedPhongRender(layer, texturesFramebuffersRenderer);

			glDisable(GL_DEPTH_TEST);
			BasicRender(layer, texturesFramebuffersRenderer);
			TexturedRender(layer, texturesFramebuffersRenderer);
			CustomShadersRender(layer, texturesFramebuffersRenderer);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenInfo.windowSize.x, screenInfo.windowSize.y);
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		assert(Globals::Components().mainFramebufferRenderer().renderer);
		Globals::Components().mainFramebufferRenderer().renderer(framebuffers.main.textureUnit - GL_TEXTURE0);
	}
}
