#include "rocketball.hpp"

#include <components/systemInfo.hpp>
#include <components/physics.hpp>
#include <components/texture.hpp>
#include <components/plane.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/camera2D.hpp>
#include <components/decoration.hpp>
#include <components/graphicsSettings.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>
#include <components/animatedTexture.hpp>

#include <globals/components.hpp>

#include <ogl/uniformsUtils.hpp>

#include <tools/Shapes2D.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>
#include <tools/playersHandler.hpp>

#include <algorithm>

namespace
{
	constexpr static float ballSize = 2.0f;
	constexpr static glm::vec2 levelHSize = { 100.0f, 60.0f };
	constexpr static float bordersHGauge = 50.0f;
	constexpr static float grappleHookRange = 15.0f;
}

namespace Levels
{
	class Rocketball::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			Globals::Components().graphicsSettings().backgroundColorF = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
			Globals::Components().graphicsSettings().defaultColorF = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().staticTextures();

			planeTextures[0] = textures.emplace("textures/plane 1.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.6f, 1.8f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			planeTextures[1] = textures.emplace("textures/alien ship 1.png");
			textures.last().translate = glm::vec2(-0.2f, 0.0f);
			textures.last().scale = glm::vec2(1.9f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			planeTextures[2] = textures.emplace("textures/plane 2.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.8f, 1.8f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			planeTextures[3] = textures.emplace("textures/alien ship 2.png");
			textures.last().translate = glm::vec2(0.0f, 0.0f);
			textures.last().scale = glm::vec2(1.45f, 1.4f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			ballTexture = textures.size();
			textures.emplace("textures/ball.png");
			textures.last().scale = glm::vec2(ballSize * 2);

			woodTexture = textures.size();
			textures.emplace("textures/wood.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(50.0f);

			playFieldTexture = textures.size();
			textures.emplace("textures/play field.jpg");

			flameAnimationTexture = textures.size();
			textures.emplace("textures/flame animation 1.jpg");
			textures.last().minFilter = GL_LINEAR;
		}

		void setAnimations()
		{
			for (auto& flameAnimatedTextureForPlayer : flameAnimatedTextureForPlayers)
			{
				flameAnimatedTextureForPlayer = Globals::Components().staticAnimatedTextures().add({ CM::Texture(flameAnimationTexture, true), { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
					AnimationData::Direction::Backward, AnimationData::Mode::Repeat, AnimationData::TextureLayout::Horizontal });
				Globals::Components().staticAnimatedTextures().last().start(true);
			}
		}

		void createBackground() const
		{
			Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 100.0f, 60.0f }),
				CM::Texture(playFieldTexture, true), Tools::Shapes2D::CreateTexCoordOfRectangle(), nullptr);
			Globals::Components().staticDecorations().last().renderLayer = RenderLayer::Background;
		}

		void createPlayers()
		{
			playersHandler.initPlayers(Tools::PlayersHandler::InitPlayerParams{}.planeTextures(planeTextures).flameTextures(flameAnimatedTextureForPlayers).gamepadForPlayer1(false).initLocationFunc(
				[](unsigned playerId, unsigned numOfPlayers) {
					const float gap = 5.0f;
					const float farPlayersDistance = gap * (numOfPlayers - 1);
					return glm::vec3(-10.0f, -farPlayersDistance / 2.0f + gap * playerId, 0.0f);
				}));
		}

		void createStationaryWalls() const
		{
			Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ bordersHGauge, levelHSize.y + bordersHGauge * 2 },
				Tools::BodyParams().position({ -levelHSize.x - bordersHGauge, 0.0f })), CM::Texture(woodTexture, true));

			Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ bordersHGauge, levelHSize.y + bordersHGauge * 2 },
				Tools::BodyParams().position({ levelHSize.x + bordersHGauge, 0.0f })), CM::Texture(woodTexture, true));

			Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ levelHSize.x + bordersHGauge * 2, bordersHGauge },
				Tools::BodyParams().position({ 0.0f, -levelHSize.y - bordersHGauge })), CM::Texture(woodTexture, true));

			Globals::Components().staticWalls().emplace(Tools::CreateBoxBody({ levelHSize.x + bordersHGauge * 2, bordersHGauge },
				Tools::BodyParams().position({ 0.0f, levelHSize.y + bordersHGauge })), CM::Texture(woodTexture, true));
		}

		void createBall()
		{
			ball = &Globals::Components().grapples().emplace(Tools::CreateDiscBody(2.0f, Tools::BodyParams().bodyType(b2_dynamicBody).density(0.02f).restitution(0.5f)), CM::Texture(ballTexture, true));
			ball->range = grappleHookRange;
		}

		void setCamera()
		{
			playersHandler.setCamera(Tools::PlayersHandler::CameraParams().projectionHSizeMin(50.0f).additionalActors([&]() {
				return ball->getOrigin2D();
			}));
		}

		void step()
		{
			playersHandler.gamepadsAutodetectionStep([](auto) { return glm::vec3(0.0f); });
			playersHandler.controlStep();
		}

	private:
		std::array<CM::Texture, 4> planeTextures;
		unsigned woodTexture = 0;
		unsigned ballTexture = 0;
		unsigned playFieldTexture = 0;
		unsigned flameAnimationTexture = 0;

		std::array<CM::AnimatedTexture, 4> flameAnimatedTextureForPlayers;

		Tools::PlayersHandler playersHandler;
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
		impl->createStationaryWalls();
		impl->createBall();
		impl->setCamera();
	}

	Rocketball::~Rocketball() = default;

	void Rocketball::step()
	{
		impl->step();
	}
}
