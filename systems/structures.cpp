#include "structures.hpp"

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>
#include <ogl/renderingHelpers.hpp>
#include <ogl/oglHelpers.hpp>

#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/textured.hpp>

#include <components/physics.hpp>
#include <components/mvp.hpp>
#include <components/renderingSetup.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/texture.hpp>
#include <components/graphicsSettings.hpp>
#include <components/framebuffers.hpp>

#include <globals/shaders.hpp>

#include <Box2D/Box2D.h>

#include <GL/glew.h>

namespace Systems
{
	Structures::Structures() = default;

	void Structures::postInit()
	{
		initGraphics();
	}

	void Structures::step()
	{
		for (const auto& wall : Globals::Components().walls())
			if (wall.step)
				wall.step();

		for (const auto& [id, wall] : Globals::Components().dynamicWalls())
			if (wall.step)
				wall.step();

		for (auto& [id, grapple] : Globals::Components().grapples())
			if (grapple.step)
				grapple.step();

		updateDynamicBuffers();
	}

	void Structures::render() const
	{
		basicRender();
		texturedRender();
		customShadersRender();
	}

	void Structures::updateStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().walls(), staticBuffers.simpleWalls, staticBuffers.texturedWalls, staticBuffers.customShadersWalls);
	}

	void Structures::initGraphics()
	{
		updateStaticBuffers();
	}

	void Structures::updateDynamicBuffers()
	{
		Tools::UpdateDynamicBuffers(Globals::Components().dynamicWalls(), dynamicBuffers.simpleWalls, dynamicBuffers.texturedWalls, dynamicBuffers.customShadersWalls);
		Tools::UpdateDynamicBuffers(Globals::Components().grapples(), dynamicBuffers.simpleGrapples, dynamicBuffers.texturedGrapples, dynamicBuffers.customShadersGrapples);
	}

	void Structures::customShadersRender() const
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

		for (const auto& buffers : staticBuffers.customShadersWalls)
			render(buffers);

		for (const auto& buffers : staticBuffers.customShadersGrapples)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers.customShadersWalls)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers.customShadersGrapples)
			render(buffers);
	}

	void Structures::texturedRender() const
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

		for (const auto& buffers : staticBuffers.texturedWalls)
			render(buffers);

		for (const auto& buffers : staticBuffers.texturedGrapples)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers.texturedWalls)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers.texturedGrapples)
			render(buffers);
	}

	void Structures::basicRender() const
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

		for (const auto& buffers : staticBuffers.simpleWalls)
			render(buffers);

		for (const auto& buffers : staticBuffers.simpleGrapples)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers.simpleWalls)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers.simpleGrapples)
			render(buffers);
	}
}
