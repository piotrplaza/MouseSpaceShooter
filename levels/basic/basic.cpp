#include "basic.hpp"

#include <components/graphicsSettings.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/camera.hpp>
#include <components/plane.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <ogl/renderingHelpers.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/gameHelpers.hpp>

namespace Levels
{
	class Basic::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			Globals::Components().graphicsSettings().defaultColor = { 0.7f, 0.7f, 0.7f, 1.0f };
			Globals::Components().mainFramebufferRenderer().renderer = Tools::StandardFullscreenRenderer(Globals::Shaders().textured());
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().textures();

			rocketPlaneTexture = textures.size();
			textures.emplace_back("textures/rocket plane.png");
			textures.back().translate = glm::vec2(0.4f, 0.0f);
			textures.back().scale = glm::vec2(1.6f, 1.8f);

			flame1AnimationTexture = textures.size();
			textures.emplace_back("textures/flame animation 1.jpg");
			textures.back().minFilter = GL_LINEAR;
		}

		void setAnimations()
		{
			flame1AnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().push_back(Components::AnimatedTexture(
				flame1AnimationTexture, { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
				AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal));
			Globals::Components().animatedTextures().back().start(true);
		}

		void createAdditionalDecorations() const
		{
		}

		void createPlayers()
		{
			player1Handler = Tools::CreatePlane(rocketPlaneTexture, flame1AnimatedTexture, { 0.0f, 0.0f }, glm::half_pi<float>());
		}

		void setCamera() const
		{
			const auto& plane = Globals::Components().planes()[player1Handler.planeId];

			Globals::Components().camera().targetProjectionHSizeF = [&]() {
				return 10.0f;
			};
			Globals::Components().camera().targetPositionF = [&]() {
				return glm::vec2(0.0f, 0.0f);
			};
		}

		void step()
		{
			const auto& mouse = Globals::Components().mouse();
			auto& player1Controls = Globals::Components().planes()[player1Handler.planeId].controls;

			player1Controls.turningDelta = mouse.getWorldSpaceDelta();
			player1Controls.autoRotation = mouse.pressing.rmb;
			player1Controls.throttling = mouse.pressing.rmb;
			player1Controls.magneticHook = mouse.pressing.xmb1;
		}

	private:
		unsigned rocketPlaneTexture = 0;
		unsigned flame1AnimationTexture = 0;

		unsigned flame1AnimatedTexture = 0;

		Tools::PlaneHandler player1Handler;
	};

	Basic::Basic():
		impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->setAnimations();
		impl->createPlayers();
		impl->setCamera();
	}

	Basic::~Basic() = default;

	void Basic::step()
	{
		impl->step();
	}
}
