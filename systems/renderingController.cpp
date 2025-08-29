#include "renderingController.hpp"

#include <components/systemInfo.hpp>
#include <components/graphicsSettings.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/renderingBuffers.hpp>
#include <components/vp.hpp>

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
#include <globals/components.hpp>

namespace
{
	void BasicPhongRender(size_t layer, auto& renderTextureClearer, const auto& staticBuffers, const auto& dynamicBuffers)
	{
		if (staticBuffers[layer].empty() && dynamicBuffers[layer].empty())
			return;

		const auto& graphicsSettings = Globals::Components().graphicsSettings();

		glProxyUseProgram(Globals::Shaders().basicPhong().getProgramId());

		const float prevForcedAlpha = Globals::Shaders().basicPhong().forcedAlpha.getValue();

		auto render = [&](const auto& buffers) {
			assert(buffers.renderable->targetTexture.isValid());
			const auto& cmTargetTexture = buffers.renderable->targetTexture;
			const auto& targetTexture = *cmTargetTexture.component;
			const auto& standardRenderMode = targetTexture.loaded.standardRenderMode;
			const auto& mainRenderTexture = Globals::Components().standardRenderTexture();
			Tools::ConditionalScopedFramebuffer csfb(!standardRenderMode || !standardRenderMode->isMainMode(), targetTexture.loaded.fbo,
				targetTexture.loaded.size, mainRenderTexture.loaded.fbo, mainRenderTexture.loaded.size);

			renderTextureClearer.clearIfFirstOfRenderTexture(cmTargetTexture);

			Globals::Shaders().basicPhong().vp(buffers.renderable->vpMatrix.component->getVP());

			buffers.draw(Globals::Shaders().basicPhong(), [&](const auto& buffers) {
				const auto modelMatrix = (buffers.renderable->modelMatrixF)();
				Globals::Shaders().basicPhong().model(modelMatrix);
				Globals::Shaders().basicPhong().normalMatrix(Globals::Components().vpDefault3D().getNormalMatrix(modelMatrix));
				Globals::Shaders().basicPhong().color(buffers.renderable->colorF.isLoaded() ? (buffers.renderable->colorF)() : graphicsSettings.defaultColorF());
				Globals::Shaders().basicPhong().ambient(buffers.renderable->params3D->ambient_);
				Globals::Shaders().basicPhong().diffuse(buffers.renderable->params3D->diffuse_);
				Globals::Shaders().basicPhong().specular(buffers.renderable->params3D->specular_);
				Globals::Shaders().basicPhong().specularFocus(buffers.renderable->params3D->specularFocus_);
				Globals::Shaders().basicPhong().specularMaterialColorFactor(buffers.renderable->params3D->specularMaterialColorFactor_);
				Globals::Shaders().basicPhong().illumination(buffers.renderable->params3D->illuminationF_.isLoaded() ? buffers.renderable->params3D->illuminationF_() : glm::vec4(0.0f));
				Globals::Shaders().basicPhong().darkColor(buffers.renderable->params3D->darkColor_.isLoaded() ? buffers.renderable->params3D->darkColor_() : graphicsSettings.backgroundColorF());
				Globals::Shaders().basicPhong().lightModelEnabled(buffers.renderable->params3D->lightModelEnabled_);
				Globals::Shaders().basicPhong().gpuSideInstancedNormalTransforms(buffers.renderable->params3D->gpuSideInstancedNormalTransforms_);
				Globals::Shaders().basicPhong().fogAmplification(buffers.renderable->params3D->fogAmplification_);
				Globals::Shaders().basicPhong().lightModelColorNormalization(buffers.renderable->params3D->lightModelColorNormalization_);
			}, [](auto&) {
				Globals::Shaders().basicPhong().forcedAlpha(!glProxyIsBlendEnabled() * 2 - 1.0f);
			}, [](auto&) {}, [](auto&) {});
		};

		for (const auto& buffers : staticBuffers[layer])
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers[layer])
			render(buffers);

		Globals::Shaders().basicPhong().forcedAlpha(prevForcedAlpha);
	}

	void TexturedPhongRender(size_t layer, auto& renderTextureClearer, const auto& staticBuffers, const auto& dynamicBuffers)
	{
		if (staticBuffers[layer].empty() && dynamicBuffers[layer].empty())
			return;

		const auto& graphicsSettings = Globals::Components().graphicsSettings();

		glProxyUseProgram(Globals::Shaders().texturedPhong().getProgramId());

		const float prevForcedAlpha = Globals::Shaders().texturedPhong().forcedAlpha.getValue();

		auto render = [&](const auto& buffers) {
			assert(buffers.renderable->targetTexture.isValid());
			const auto& cmTargetTexture = buffers.renderable->targetTexture;
			const auto& targetTexture = *cmTargetTexture.component;
			const auto& standardRenderMode = targetTexture.loaded.standardRenderMode;
			const auto& mainRenderTexture = Globals::Components().standardRenderTexture();
			Tools::ConditionalScopedFramebuffer csfb(!standardRenderMode || !standardRenderMode->isMainMode(), targetTexture.loaded.fbo,
				targetTexture.loaded.size, mainRenderTexture.loaded.fbo, mainRenderTexture.loaded.size);

			renderTextureClearer.clearIfFirstOfRenderTexture(cmTargetTexture);

			Globals::Shaders().texturedPhong().vp(buffers.renderable->vpMatrix.component->getVP());

			buffers.draw(Globals::Shaders().texturedPhong(), [&](const auto& buffers) {
				const auto modelMatrix = (buffers.renderable->modelMatrixF)();
				Globals::Shaders().texturedPhong().model(modelMatrix);
				Globals::Shaders().texturedPhong().normalMatrix(Globals::Components().vpDefault3D().getNormalMatrix(modelMatrix));
				Globals::Shaders().texturedPhong().color(buffers.renderable->colorF.isLoaded() ? (buffers.renderable->colorF)() : graphicsSettings.defaultColorF());
				Globals::Shaders().texturedPhong().ambient(buffers.renderable->params3D->ambient_);
				Globals::Shaders().texturedPhong().diffuse(buffers.renderable->params3D->diffuse_);
				Globals::Shaders().texturedPhong().specular(buffers.renderable->params3D->specular_);
				Globals::Shaders().texturedPhong().specularFocus(buffers.renderable->params3D->specularFocus_);
				Globals::Shaders().texturedPhong().specularMaterialColorFactor(buffers.renderable->params3D->specularMaterialColorFactor_);
				Globals::Shaders().texturedPhong().illumination(buffers.renderable->params3D->illuminationF_.isLoaded() ? buffers.renderable->params3D->illuminationF_() : glm::vec4(0.0f));
				Globals::Shaders().texturedPhong().darkColor(buffers.renderable->params3D->darkColor_.isLoaded() ? buffers.renderable->params3D->darkColor_() : graphicsSettings.backgroundColorF());
				Globals::Shaders().texturedPhong().lightModelEnabled(buffers.renderable->params3D->lightModelEnabled_);
				Globals::Shaders().texturedPhong().alphaDiscardTreshold(buffers.renderable->params3D->alphaDiscardTreshold_);
				Globals::Shaders().texturedPhong().gpuSideInstancedNormalTransforms(buffers.renderable->params3D->gpuSideInstancedNormalTransforms_);
				Globals::Shaders().texturedPhong().fogAmplification(buffers.renderable->params3D->fogAmplification_);
				Globals::Shaders().texturedPhong().lightModelColorNormalization(buffers.renderable->params3D->lightModelColorNormalization_);
				Tools::PrepareTexturedRender(Globals::Shaders().texturedPhong(), buffers.renderable->texture);
			}, [](auto&) {
				Globals::Shaders().texturedPhong().forcedAlpha(!glProxyIsBlendEnabled() * 2 - 1.0f);
			}, [](auto&) {}, [](auto&) {});
		};

		for (const auto& buffers : staticBuffers[layer])
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers[layer])
			render(buffers);

		Globals::Shaders().texturedPhong().forcedAlpha(prevForcedAlpha);
	}

	void BasicRender(size_t layer, auto& renderTextureClearer, const auto& staticBuffers, const auto& dynamicBuffers)
	{
		if (staticBuffers[layer].empty() && dynamicBuffers[layer].empty())
			return;

		const auto& graphicsSettings = Globals::Components().graphicsSettings();

		glProxyUseProgram(Globals::Shaders().basic().getProgramId());

		const float prevForcedAlpha = Globals::Shaders().basic().forcedAlpha.getValue();

		auto render = [&](const auto& buffers) {
			assert(buffers.renderable->targetTexture.isValid());
			const auto& cmTargetTexture = buffers.renderable->targetTexture;
			const auto& targetTexture = *cmTargetTexture.component;
			const auto& standardRenderMode = targetTexture.loaded.standardRenderMode;
			const auto& mainRenderTexture = Globals::Components().standardRenderTexture();
			Tools::ConditionalScopedFramebuffer csfb(!standardRenderMode || !standardRenderMode->isMainMode(), targetTexture.loaded.fbo,
				targetTexture.loaded.size, mainRenderTexture.loaded.fbo, mainRenderTexture.loaded.size);

			renderTextureClearer.clearIfFirstOfRenderTexture(cmTargetTexture);

			Globals::Shaders().basic().vp(buffers.renderable->vpMatrix.component->getVP());

			buffers.draw(Globals::Shaders().basic(), [&](const auto& buffers) {
				Globals::Shaders().basic().model((buffers.renderable->modelMatrixF)());
				Globals::Shaders().basic().color(buffers.renderable->colorF.isLoaded() ? (buffers.renderable->colorF)() : graphicsSettings.defaultColorF());
			}, [](auto&) {
				Globals::Shaders().basic().forcedAlpha(!glProxyIsBlendEnabled() * 2 - 1.0f);
			}, [](auto&) {}, [](auto&) {});
		};

		for (const auto& buffers : staticBuffers[layer])
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers[layer])
			render(buffers);

		Globals::Shaders().basic().forcedAlpha(prevForcedAlpha);
	}

	void TexturedRender(size_t layer, auto& renderTextureClearer, const auto& staticBuffers, const auto& dynamicBuffers)
	{
		if (staticBuffers[layer].empty() && dynamicBuffers[layer].empty())
			return;

		const auto& graphicsSettings = Globals::Components().graphicsSettings();

		glProxyUseProgram(Globals::Shaders().textured().getProgramId());

		const float prevForcedAlpha = Globals::Shaders().textured().forcedAlpha.getValue();

		auto render = [&](const auto& buffers) {
			assert(buffers.renderable->targetTexture.isValid());
			const auto& cmTargetTexture = buffers.renderable->targetTexture;
			const auto& targetTexture = *cmTargetTexture.component;
			const auto& standardRenderMode = targetTexture.loaded.standardRenderMode;
			const auto& mainRenderTexture = Globals::Components().standardRenderTexture();
			Tools::ConditionalScopedFramebuffer csfb(!standardRenderMode || !standardRenderMode->isMainMode(), targetTexture.loaded.fbo,
				targetTexture.loaded.size, mainRenderTexture.loaded.fbo, mainRenderTexture.loaded.size);

			renderTextureClearer.clearIfFirstOfRenderTexture(cmTargetTexture);

			Globals::Shaders().textured().vp(buffers.renderable->vpMatrix.component->getVP());

			buffers.draw(Globals::Shaders().textured(), [&](const auto& buffers) {
				Globals::Shaders().textured().model((buffers.renderable->modelMatrixF)());
				Globals::Shaders().textured().visibilityCenter((buffers.renderable->originF)());
				Globals::Shaders().textured().color(buffers.renderable->colorF.isLoaded() ? buffers.renderable->colorF() : graphicsSettings.defaultColorF());
				Tools::PrepareTexturedRender(Globals::Shaders().textured(), buffers.renderable->texture);
			}, [](auto&) {
				Globals::Shaders().textured().forcedAlpha(!glProxyIsBlendEnabled() * 2 - 1.0f);
			}, [](auto&) {}, [](auto&) {});
		};

		for (const auto& buffers : staticBuffers[layer])
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers[layer])
			render(buffers);

		Globals::Shaders().textured().forcedAlpha(prevForcedAlpha);
	}

	void CustomShadersRender(size_t layer, auto& renderTextureClearer, const auto& staticBuffers, const auto& dynamicBuffers)
	{
		if (staticBuffers[layer].empty() && dynamicBuffers[layer].empty())
			return;

		auto render = [&](const auto& buffers) {
			assert(buffers.renderable->targetTexture.isValid());
			const auto& cmTargetTexture = buffers.renderable->targetTexture;
			const auto& targetTexture = *cmTargetTexture.component;
			const auto& standardRenderMode = targetTexture.loaded.standardRenderMode;
			const auto& mainRenderTexture = Globals::Components().standardRenderTexture();
			Tools::ConditionalScopedFramebuffer csfb(!standardRenderMode || !standardRenderMode->isMainMode(), targetTexture.loaded.fbo,
				targetTexture.loaded.size, mainRenderTexture.loaded.fbo, mainRenderTexture.loaded.size);

			renderTextureClearer.clearIfFirstOfRenderTexture(cmTargetTexture);

			assert(buffers.renderable->customShadersProgram);
			glProxyUseProgram(*buffers.renderable->customShadersProgram);

			buffers.draw(*buffers.renderable->customShadersProgram, [](auto&) {}, [](auto&) {}, [](auto&) {}, [](auto&) {});
		};

		for (const auto& buffers : staticBuffers[layer])
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers[layer])
			render(buffers);
	}

	void TransformFeedbackRender()
	{
		const auto& staticTFBuffers = Globals::Components().renderingBuffers().staticTFBuffers;
		const auto& dynamicTFBuffers = Globals::Components().renderingBuffers().dynamicTFBuffers;

		if (staticTFBuffers.empty() && dynamicTFBuffers.empty())
			return;

		auto render = [&](const auto& sourceBuffers, const auto& tfBuffers) {
			assert(sourceBuffers.renderable == tfBuffers.renderable);
			const auto& renderable = *sourceBuffers.renderable;
			assert(renderable.tfShaderProgram);
			assert(renderable.loaded.buffers == &sourceBuffers);
			assert(renderable.loaded.tfBuffers == &tfBuffers);
			glProxyUseProgram(*renderable.tfShaderProgram);
			sourceBuffers.draw(*renderable.tfShaderProgram, [&](const auto& subBuffers) {
				renderable.loaded.tfBuffers->bindActiveTFBuffers();
				glBeginTransformFeedback(subBuffers.renderable->drawMode);
			}, [](auto&) {}, [](auto&) {
				glEndTransformFeedback();
			}, [](auto&) {}, true);
			glFlush();
			renderable.loaded.buffers->swapActiveBuffers(*renderable.loaded.tfBuffers);
		};

		glEnable(GL_RASTERIZER_DISCARD);

		for (const auto& tfBuffers : staticTFBuffers)
		{
			const auto& sourceBuffers = *tfBuffers.renderable->loaded.buffers;
			render(sourceBuffers, tfBuffers);
		}

		for (const auto& [id, tfBuffers] : dynamicTFBuffers)
		{
			const auto& sourceBuffers = *tfBuffers.renderable->loaded.buffers;
			render(sourceBuffers, tfBuffers);
		}

		glDisable(GL_RASTERIZER_DISCARD);
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
		const auto& graphicsSettings = Globals::Components().graphicsSettings();
		const auto clearColor = graphicsSettings.backgroundColorF();
		const auto& screenInfo = Globals::Components().systemInfo().screen;
		const auto& mainRenderTexture = Globals::Components().standardRenderTexture();
		const auto& staticBuffers = Globals::Components().renderingBuffers().staticBuffers;
		const auto& dynamicBuffers = Globals::Components().renderingBuffers().dynamicBuffers;
		const auto& staticOfflineBuffers = Globals::Components().renderingBuffers().staticOfflineBuffers;
		const auto& dynamicOfflineBuffers = Globals::Components().renderingBuffers().dynamicOfflineBuffers;

		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

		if (graphicsSettings.forcedDepthTest)
			glProxySetDepthTest(*graphicsSettings.forcedDepthTest);
		else if (graphicsSettings.force3D)
			glProxySetDepthTest(true);

		glProxySetCullFace(graphicsSettings.cullFace);
		glProxySetPointSmooth(graphicsSettings.pointSmooth);
		glProxySetLineSmooth(graphicsSettings.lineSmooth);

		glProxyPointSize(graphicsSettings.pointSize);
		glProxyLineWidth(graphicsSettings.lineWidth);

		Tools::Lights3DSetup(Globals::Shaders().basicPhong());
		Tools::Lights3DSetup(Globals::Shaders().texturedPhong());

		const auto viewPos = Globals::Components().vpDefault3D().getViewPos();
		Globals::Shaders().basicPhong().viewPos(viewPos);
		Globals::Shaders().texturedPhong().viewPos(viewPos);

		Globals::Shaders().frameSetup();

		for (size_t layer = 0; layer < (size_t)RenderLayer::COUNT; ++layer)
		{
			CustomRenderTexturesRenderer customRenderTexturesRenderer;

			if (!graphicsSettings.forcedDepthTest)
				glProxySetDepthTest(true);

			BasicPhongRender(layer, customRenderTexturesRenderer, staticOfflineBuffers.basicPhong, dynamicOfflineBuffers.basicPhong);
			TexturedPhongRender(layer, customRenderTexturesRenderer, staticOfflineBuffers.texturedPhong, dynamicOfflineBuffers.texturedPhong);

			if (!graphicsSettings.forcedDepthTest)
				glProxySetDepthTest(graphicsSettings.force3D);

			BasicRender(layer, customRenderTexturesRenderer, staticOfflineBuffers.basic, dynamicOfflineBuffers.basic);
			TexturedRender(layer, customRenderTexturesRenderer, staticOfflineBuffers.textured, dynamicOfflineBuffers.textured);
			CustomShadersRender(layer, customRenderTexturesRenderer, staticOfflineBuffers.customShaders, dynamicOfflineBuffers.customShaders);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, mainRenderTexture.loaded.fbo);
		glViewport(0, 0, mainRenderTexture.loaded.size.x, mainRenderTexture.loaded.size.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (size_t layer = 0; layer < (size_t)RenderLayer::COUNT; ++layer)
		{
			StandardRenderTexturesRenderer standardRenderTexturesRenderer(Globals::Shaders().textured());

			if (!graphicsSettings.forcedDepthTest)
				glProxySetDepthTest(true);

			BasicPhongRender(layer, standardRenderTexturesRenderer, staticBuffers.basicPhong, dynamicBuffers.basicPhong);
			TexturedPhongRender(layer, standardRenderTexturesRenderer, staticBuffers.texturedPhong, dynamicBuffers.texturedPhong);

			if (!graphicsSettings.forcedDepthTest)
				glProxySetDepthTest(graphicsSettings.force3D);

			BasicRender(layer, standardRenderTexturesRenderer, staticBuffers.basic, dynamicBuffers.basic);
			TexturedRender(layer, standardRenderTexturesRenderer, staticBuffers.textured, dynamicBuffers.textured);
			CustomShadersRender(layer, standardRenderTexturesRenderer, staticBuffers.customShaders, dynamicBuffers.customShaders);
		}

		TransformFeedbackRender();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenInfo.windowSize.x, screenInfo.windowSize.y);
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		assert(Globals::Components().mainFramebufferRenderer().renderer);
		Globals::Components().mainFramebufferRenderer().renderer(mainRenderTexture.loaded.textureObject);
	}
}
