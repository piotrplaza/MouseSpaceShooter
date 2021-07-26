#include "renderingController.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <ogl/oglProxy.hpp>
#include <ogl/renderingHelpers.hpp>

#include <globals.hpp>

#include <components/screenInfo.hpp>
#include <components/graphicsSettings.hpp>
#include <components/framebuffers.hpp>
#include <components/mouseState.hpp>
#include <components/physics.hpp>

#include <systems/walls.hpp>
#include <systems/players.hpp>
#include <systems/temporaries.hpp>
#include <systems/decorations.hpp>

#include <tools/utility.hpp>

namespace Systems
{
	RenderingController::RenderingController()
	{
		initGraphics();
	}

	void RenderingController::initGraphics()
	{
		texturedShadersProgram = std::make_unique<Shaders::Programs::Textured>();
	}

	void RenderingController::render()
	{
		const auto& screenInfo = Globals::Components().screenInfo();
		auto& framebuffers = Globals::Components().framebuffers();

		{
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.main.fbo);
			glViewport(0, 0, framebuffers.main.size.x, framebuffers.main.size.y);
			const glm::vec4& clearColor = Globals::Components().graphicsSettings().clearColor;
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
		}

		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, screenInfo.windowSize.x, screenInfo.windowSize.y);
			const glm::vec4& clearColor = Globals::Components().graphicsSettings().clearColor;
			glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			vp = glm::perspective(glm::radians(28.0f), (float)screenInfo.windowSize.x / screenInfo.windowSize.y, 1.0f, 10.0f);
			model = glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, -4.0f });
			model = glm::rotate(model, angle, { 1.0f, 1.0f, 1.0f });
			model = glm::scale(model, { (float)screenInfo.windowSize.x / screenInfo.windowSize.y, 1.0f, 1.0f });

			glDisable(GL_BLEND);
			Tools::TexturedScreenRender(*texturedShadersProgram, framebuffers.main.textureUnit - GL_TEXTURE0, [&]()
				{
					texturedShadersProgram->vpUniform.setValue(vp);
					texturedShadersProgram->modelUniform.setValue(model);
				}, [&]()
				{
					const float quakeIntensity = 0.005f * Globals::Components().shockwaves().size();
					const glm::vec2 quakeIntensityXY = screenInfo.windowSize.x > screenInfo.windowSize.y
						? glm::vec2(quakeIntensity, quakeIntensity * (float)screenInfo.windowSize.x / screenInfo.windowSize.y)
						: glm::vec2(quakeIntensity * (float)screenInfo.windowSize.y / screenInfo.windowSize.x, quakeIntensity);


					const glm::vec3 p1 = { -1.0f - Tools::Random(0.0f, quakeIntensityXY.x), -1.0f - Tools::Random(0.0f, quakeIntensityXY.y), 0.0f };
					const glm::vec3 p2 = { 1.0f + Tools::Random(0.0f, quakeIntensityXY.x), p1.y, 0.0f };
					const glm::vec3 p3 = { p1.x, 1.0f + Tools::Random(0.0f, quakeIntensityXY.y), 0.0f };
					const glm::vec3 p4 = { p2.x, p3.y, 0.0f };

					return std::array<glm::vec3, 6>{ p1, p2, p3, p3, p2, p4 };
				});
			glEnable(GL_BLEND);

			const float angleDelta = Globals::Components().physics().frameDuration * 2.0f;

			if (angle == 0.0f && Globals::Components().mouseState().xmb2)
			{
				angle = angleDelta;
			}
			if (angle > 0.0f && angle <= glm::two_pi<float>())
			{
				angle += angleDelta;
			}
			if (angle > glm::two_pi<float>())
			{
				angle = 0.0f;
			}
		}
	}
}
