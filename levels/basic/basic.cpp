#include "basic.hpp"

#include <components/graphicsSettings.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/camera.hpp>
#include <components/plane.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>
#include <components/staticWall.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <ogl/renderingHelpers.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/gameHelpers.hpp>
#include <tools/playersHandler.hpp>

#include <iostream>
using namespace std;

namespace Levels
{
	class Basic::Impl
	{
	public:
		void setup()
		{
			Globals::Components().graphicsSettings().defaultColor = { 0.7f, 0.7f, 0.7f, 1.0f };
			Globals::Components().mainFramebufferRenderer().renderer = Tools::StandardFullscreenRenderer(Globals::Shaders().textured());

			playersHandler.initPlayers(planeTextures, flameAnimatedTextureForPlayers, false,
				[this](unsigned player, unsigned numOfPlayers) {
					const float gap = 5.0f;
					const float farPlayersDistance = gap * (numOfPlayers - 1);
					return glm::vec3(-10.0f, -farPlayersDistance / 2.0f + gap * player, 0.0f);
				});

			for (const auto& plane : Globals::Components().planes())
				cout << plane.body->GetMass() << endl;
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().textures();

			planeTextures[0] = textures.size();
			textures.emplace("textures/plane 1.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.6f, 1.8f);
			textures.last().minFilter = GL_LINEAR;

			planeTextures[1] = textures.size();
			textures.emplace("textures/alien ship 1.png");
			textures.last().translate = glm::vec2(-0.2f, 0.0f);
			textures.last().scale = glm::vec2(1.9f);
			textures.last().minFilter = GL_LINEAR;

			planeTextures[2] = textures.size();
			textures.emplace("textures/plane 2.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.8f, 1.8f);
			textures.last().minFilter = GL_LINEAR;

			planeTextures[3] = textures.size();
			textures.emplace("textures/alien ship 2.png");
			textures.last().translate = glm::vec2(0.0f, 0.0f);
			textures.last().scale = glm::vec2(1.45f, 1.4f);
			textures.last().minFilter = GL_LINEAR;

			flameAnimationTexture = textures.size();
			textures.emplace("textures/flame animation 1.jpg");
			textures.last().minFilter = GL_LINEAR;
		}

		void setAnimations()
		{
			for (unsigned& flameAnimatedTextureForPlayer : flameAnimatedTextureForPlayers)
			{
				flameAnimatedTextureForPlayer = Globals::Components().animatedTextures().size();
				Globals::Components().animatedTextures().add({ flameAnimationTexture, { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
					AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal });
				Globals::Components().animatedTextures().last().start(true);
			}
		}

		void createWalls()
		{
			auto& staticWalls = Globals::Components().staticWalls();
			staticWalls.emplace(Tools::CreateBoxBody({ 1.0f, 1.0f }));
		}

		void setCamera() const
		{
			playersHandler.setCamera(Tools::PlayersHandler::CameraParams().projectionHSizeMin([]() { return 10.0f; }).scalingFactor(0.7f));
		}

		void step()
		{
			playersHandler.controlStep();
		}

	private:
		std::array<unsigned, 4> planeTextures{ 0 };
		unsigned flameAnimationTexture = 0;

		std::array<unsigned, 4> flameAnimatedTextureForPlayers{ 0 };

		Tools::PlayersHandler playersHandler;
	};

	Basic::Basic():
		impl(std::make_unique<Impl>())
	{
		impl->loadTextures();
		impl->setAnimations();
		impl->setCamera();
		impl->setup();
		impl->createWalls();
	}

	Basic::~Basic() = default;

	void Basic::step()
	{
		impl->step();
	}
}
