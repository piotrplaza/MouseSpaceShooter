#include "rocketball.hpp"

#include <algorithm>

#include <globals.hpp>

#include <components/screenInfo.hpp>
#include <components/physics.hpp>
#include <components/textureDef.hpp>
#include <components/player.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/camera.hpp>
#include <components/decoration.hpp>
#include <components/graphicsSettings.hpp>
#include <components/mouseState.hpp>

#include <ogl/uniformControllers.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>

namespace Levels
{
	class Rocketball::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			using namespace Globals::Components;

			graphicsSettings.clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			graphicsSettings.defaultColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		void setTextures()
		{
			using namespace Globals::Components;

			rocketPlaneTexture = texturesDef.size();
			texturesDef.emplace_back("textures/rocket plane.png");
			texturesDef.back().translate = glm::vec2(-0.35f, -0.5f);
			texturesDef.back().scale = glm::vec2(1.7f);

			woodTexture = texturesDef.size();
			texturesDef.emplace_back("textures/wood.jpg", GL_MIRRORED_REPEAT);
			texturesDef.back().scale = glm::vec2(50.0f);

			orbTexture = texturesDef.size();
			texturesDef.emplace_back("textures/orb.png");
			texturesDef.back().translate = glm::vec2(-0.5f);
			texturesDef.back().scale = glm::vec2(4.0f);

			playFieldTexture = texturesDef.size();
			texturesDef.emplace_back("textures/play field.jpg");

			flameAnimation1Texture = texturesDef.size();
			texturesDef.emplace_back("textures/flame animation 1.jpg");
			texturesDef.back().minFilter = GL_LINEAR;
		}

		void setBackground() const
		{
			using namespace Globals::Components;

			backgroundDecorations.emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 100.0f, 60.0f }), playFieldTexture);
			backgroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();
		}

		void setPlayers()
		{
			player1Handler = Tools::CreatePlayerPlane(rocketPlaneTexture, flameAnimation1Texture);
		}

		void setStaticWalls() const
		{
			using namespace Globals::Components;

			const glm::vec2 levelHSize = { 100.0f, 60.0f };
			const float bordersHGauge = 50.0f;

			staticWalls.emplace_back(Tools::CreateBoxBody({ -levelHSize.x - bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize.y + bordersHGauge * 2 }), woodTexture);

			staticWalls.emplace_back(Tools::CreateBoxBody({ levelHSize.x + bordersHGauge, 0.0f },
				{ bordersHGauge, levelHSize.y + bordersHGauge * 2 }), woodTexture);

			staticWalls.emplace_back(Tools::CreateBoxBody({ 0.0f, -levelHSize.y - bordersHGauge },
				{ levelHSize.x + bordersHGauge * 2, bordersHGauge }), woodTexture);

			staticWalls.emplace_back(Tools::CreateBoxBody({ 0.0f, levelHSize.y + bordersHGauge },
				{ levelHSize.x + bordersHGauge * 2, bordersHGauge }), woodTexture);
		}

		void setGrapples()
		{
			using namespace Globals::Components;

			ball = &grapples.emplace_back(Tools::CreateCircleBody({ 0.0f, 0.0f }, 2.0f, b2_dynamicBody, 0.02f, 0.5f), 15.0f,
				orbTexture);
		}

		void setCamera() const
		{
			using namespace Globals::Components;

			const auto& player = players[player1Handler.playerId];

			camera.targetProjectionHSizeF = [&]() {
				camera.projectionTransitionFactor = physics.frameTime * 6;
				return 30.0f + glm::distance(player.getCenter(), ball->getCenter()) * 0.3f;
			};
			camera.targetPositionF = [&]() {
				camera.positionTransitionFactor = physics.frameTime * 6;
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
		unsigned flameAnimation1Texture = 0;

		Tools::PlayerPlaneHandler player1Handler;
		Components::Grapple* ball = nullptr;
	};

	Rocketball::Rocketball(): impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->setTextures();
		impl->setBackground();
		impl->setPlayers();
		impl->setStaticWalls();
		impl->setGrapples();
		impl->setCamera();
	}

	Rocketball::~Rocketball() = default;

	void Rocketball::step()
	{
		impl->step();
	}
}
