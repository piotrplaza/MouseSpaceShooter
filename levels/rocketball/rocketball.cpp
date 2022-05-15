#include "rocketball.hpp"

#include <components/screenInfo.hpp>
#include <components/physics.hpp>
#include <components/texture.hpp>
#include <components/player.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/camera.hpp>
#include <components/decoration.hpp>
#include <components/graphicsSettings.hpp>
#include <components/mouseState.hpp>
#include <components/animatedTexture.hpp>

#include <globals/components.hpp>

#include <ogl/uniformControllers.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>

#include <algorithm>

namespace Levels
{
	class Rocketball::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			Globals::Components().graphicsSettings().clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			Globals::Components().graphicsSettings().defaultColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().textures();

			rocketPlaneTexture = textures.size();
			textures.emplace_back("textures/rocket plane.png");
			textures.back().translate = glm::vec2(0.4f, 0.0f);
			textures.back().scale = glm::vec2(1.6f, 1.8f);

			woodTexture = textures.size();
			textures.emplace_back("textures/wood.jpg", GL_MIRRORED_REPEAT);
			textures.back().scale = glm::vec2(50.0f);

			orbTexture = textures.size();
			textures.emplace_back("textures/orb.png");
			textures.back().scale = glm::vec2(4.0f);

			playFieldTexture = textures.size();
			textures.emplace_back("textures/play field.jpg");

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

		void createBackground() const
		{
			Globals::Components().backgroundDecorations().emplace_back(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 100.0f, 60.0f }),
				TCM::Texture(playFieldTexture), Tools::CreateTexCoordOfRectangle());
		}

		void createPlayers()
		{
			player1Handler = Tools::CreatePlayerPlane(rocketPlaneTexture, flame1AnimatedTexture, { -10.0f, 0.0f });
			Globals::Components().players()[1].connectIfApproaching = true;
		}

		void createStaticWalls() const
		{
			const glm::vec2 levelHSize = { 100.0f, 60.0f };
			const float bordersHGauge = 50.0f;

			Globals::Components().walls().emplace_back(Tools::CreateBoxBody({ -levelHSize.x - bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize.y + bordersHGauge * 2 }), TCM::Texture(woodTexture));

			Globals::Components().walls().emplace_back(Tools::CreateBoxBody({ levelHSize.x + bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize.y + bordersHGauge * 2 }), TCM::Texture(woodTexture));

			Globals::Components().walls().emplace_back(Tools::CreateBoxBody({ 0.0f, -levelHSize.y - bordersHGauge },
				{ levelHSize.x + bordersHGauge * 2, bordersHGauge }), TCM::Texture(woodTexture));

			Globals::Components().walls().emplace_back(Tools::CreateBoxBody({ 0.0f, levelHSize.y + bordersHGauge },
				{ levelHSize.x + bordersHGauge * 2, bordersHGauge }), TCM::Texture(woodTexture));
		}

		void createGrapples()
		{
			ball = &Globals::Components().grapples().emplace_back(Tools::CreateCircleBody({ 0.0f, 0.0f }, 2.0f, b2_dynamicBody, 0.02f, 0.5f), 15.0f,
				TCM::Texture(orbTexture));
		}

		void setCamera() const
		{
			const auto& player = Globals::Components().players()[player1Handler.playerId];

			Globals::Components().camera().targetProjectionHSizeF = [&]() {
				Globals::Components().camera().projectionTransitionFactor = Globals::Components().physics().frameDuration * 6;
				return 30.0f + glm::distance(player.getCenter(), ball->getCenter()) * 0.3f;
			};
			Globals::Components().camera().targetPositionF = [&]() {
				Globals::Components().camera().positionTransitionFactor = Globals::Components().physics().frameDuration * 6;
				return (player.getCenter() + ball->getCenter()) * 0.5f;
			};
		}

		void step()
		{
		}

	private:
		unsigned rocketPlaneTexture = 0;
		unsigned woodTexture = 0;
		unsigned orbTexture = 0;
		unsigned playFieldTexture = 0;
		unsigned flame1AnimationTexture = 0;

		unsigned flame1AnimatedTexture = 0;

		Tools::PlayerPlaneHandler player1Handler;
		Components::Grapple* ball = nullptr;
	};

	Rocketball::Rocketball():
		impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->setAnimations();
		impl->createBackground();
		impl->createPlayers();
		impl->createStaticWalls();
		impl->createGrapples();
		impl->setCamera();
	}

	Rocketball::~Rocketball() = default;

	void Rocketball::step()
	{
		impl->step();
	}
}
