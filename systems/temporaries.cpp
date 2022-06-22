#include "temporaries.hpp"

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>
#include <ogl/renderingHelpers.hpp>
#include <ogl/oglHelpers.hpp>

#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/textured.hpp>

#include <components/missile.hpp>
#include <components/decoration.hpp>
#include <components/renderingSetup.hpp>
#include <components/mvp.hpp>
#include <components/graphicsSettings.hpp>
#include <components/framebuffers.hpp> 

#include <globals/shaders.hpp>

namespace Systems
{
	Temporaries::Temporaries() = default;

	void Temporaries::step()
	{
		for (auto& [id, missile] : Globals::Components().missiles())
			if (missile.step)
				missile.step();

		updateDynamicBuffers();
	}

	void Temporaries::render() const
	{
		basicRender(simpleRocketsBuffers);
		texturedRender(texturedRocketsBuffers);
		customShadersRender(customShaderRocketsBuffers);
	}

	void Temporaries::updateDynamicBuffers()
	{
		Tools::UpdateDynamicBuffers(Globals::Components().missiles(), simpleRocketsBuffers,
			texturedRocketsBuffers, customShaderRocketsBuffers);
	}

	void Temporaries::customShadersRender(const std::unordered_map<ComponentId, Buffers::GenericBuffers>& buffers) const
	{
		TexturesFramebuffersRenderer texturesFramebuffersRenderer(Globals::Shaders().textured());

		for (const auto& [id, currentBuffers] : buffers)
		{
			const auto& lowResSubBuffers = Globals::Components().framebuffers().getSubBuffers(currentBuffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(currentBuffers.resolutionMode != ResolutionMode::Normal, lowResSubBuffers.fbo,
				lowResSubBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

			texturesFramebuffersRenderer.clearIfFirstOfMode(currentBuffers.resolutionMode);

			assert(currentBuffers.customShadersProgram);
			glUseProgram_proxy(*currentBuffers.customShadersProgram);

			currentBuffers.draw(*currentBuffers.customShadersProgram, [](auto&) {});
		}
	}

	void Temporaries::texturedRender(const std::unordered_map<ComponentId, Buffers::GenericBuffers>& buffers) const
	{
		glUseProgram_proxy(Globals::Shaders().textured().getProgramId());
		Globals::Shaders().textured().vp(Globals::Components().mvp().getVP());
		Globals::Shaders().textured().color(Globals::Components().graphicsSettings().defaultColor);

		TexturesFramebuffersRenderer texturesFramebuffersRenderer(Globals::Shaders().textured());

		for (const auto& [id, currentBuffers] : buffers)
		{
			const auto& lowResSubBuffers = Globals::Components().framebuffers().getSubBuffers(currentBuffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(currentBuffers.resolutionMode != ResolutionMode::Normal, lowResSubBuffers.fbo,
				lowResSubBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

			texturesFramebuffersRenderer.clearIfFirstOfMode(currentBuffers.resolutionMode);

			currentBuffers.draw(Globals::Shaders().textured(), [](const auto& buffers) {
				Globals::Shaders().textured().model(buffers.modelMatrixF ? buffers.modelMatrixF() : glm::mat4(1.0f));
				Tools::PrepareTexturedRender(Globals::Shaders().textured(), buffers, buffers.texture);
				});
		}
	}

	void Temporaries::basicRender(const std::unordered_map<ComponentId, Buffers::GenericBuffers>& buffers) const
	{
		glUseProgram_proxy(Globals::Shaders().basic().getProgramId());
		Globals::Shaders().basic().vp(Globals::Components().mvp().getVP());
		Globals::Shaders().basic().color(Globals::Components().graphicsSettings().defaultColor);

		TexturesFramebuffersRenderer texturesFramebuffersRenderer(Globals::Shaders().textured());

		for (const auto& [id, currentBuffers] : buffers)
		{
			const auto& lowResSubBuffers = Globals::Components().framebuffers().getSubBuffers(currentBuffers.resolutionMode);
			Tools::ConditionalScopedFramebuffer csfb(currentBuffers.resolutionMode != ResolutionMode::Normal, lowResSubBuffers.fbo,
				lowResSubBuffers.size, Globals::Components().framebuffers().main.fbo, Globals::Components().framebuffers().main.size);

			texturesFramebuffersRenderer.clearIfFirstOfMode(currentBuffers.resolutionMode);

			currentBuffers.draw(Globals::Shaders().basic().getProgramId(), [](const auto& buffers) {
				Globals::Shaders().basic().model(buffers.modelMatrixF ? buffers.modelMatrixF() : glm::mat4(1.0f));
				});
		}
	}
}
