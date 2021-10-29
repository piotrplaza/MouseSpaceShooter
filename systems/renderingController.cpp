#include "renderingController.hpp"

#include <globals.hpp>

#include <components/screenInfo.hpp>
#include <components/graphicsSettings.hpp>
#include <components/framebuffers.hpp>
#include <components/mainFramebufferRenderer.hpp>

#include <systems/walls.hpp>
#include <systems/players.hpp>
#include <systems/temporaries.hpp>
#include <systems/decorations.hpp>

#include <ogl/renderingHelpers.hpp>

#include <ogl/shaders/textured.hpp>

namespace Systems
{
	RenderingController::RenderingController() = default;

	void RenderingController::postInit() const
	{
		initGraphics();
	}

	void RenderingController::render() const
	{
		const auto& screenInfo = Globals::Components().screenInfo();
		auto& framebuffers = Globals::Components().framebuffers();
		const glm::vec4& clearColor = Globals::Components().graphicsSettings().clearColor;

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.main.fbo);
		glViewport(0, 0, framebuffers.main.size.x, framebuffers.main.size.y);
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Globals::Systems().decorations().renderBackground();
		Globals::Systems().decorations().renderFarMidground();
		Globals::Systems().walls().render();
		Globals::Systems().players().render();
		Globals::Systems().temporaries().render();
		Globals::Systems().decorations().renderMidground();
		Globals::Systems().decorations().renderNearMidground();
		Globals::Systems().decorations().renderForeground();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenInfo.windowSize.x, screenInfo.windowSize.y);
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		assert(Globals::Components().mainFramebufferRenderer().renderer);
		Globals::Components().mainFramebufferRenderer().renderer(framebuffers.main.textureUnit - GL_TEXTURE0);
	}

	void RenderingController::initGraphics() const
	{
		Globals::Components().mainFramebufferRenderer().renderer = Tools::StandardFullscreenRenderer(Globals::Shaders().textured());
	}
}
