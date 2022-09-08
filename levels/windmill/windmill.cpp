#include "windmill.hpp"

#include <components/graphicsSettings.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/camera.hpp>
#include <components/plane.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>
#include <components/wall.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <ogl/renderingHelpers.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/gameHelpers.hpp>
#include <tools/b2Helpers.hpp>

namespace Levels
{
	class Windmill::Impl
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

			woodTexture = textures.size();
			textures.emplace_back("textures/wood.jpg", GL_MIRRORED_REPEAT);
			textures.back().scale = glm::vec2(5.0f);

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
			player1Id = Tools::CreatePlane(rocketPlaneTexture, flame1AnimatedTexture, { 5.0f, 5.0f }, glm::half_pi<float>());
		}

		void setCamera() const
		{
			const auto& plane = Globals::Components().planes()[player1Id];

			Globals::Components().camera().targetProjectionHSizeF = [&]() {
				return 10.0f;
			};
			Globals::Components().camera().targetPositionF = [&]() {
				return glm::vec2(0.0f, 0.0f);
			};
		}

		void createWindmill()
		{
			const float armLength = 5.0f;
			const float armOverlap = 2.0f;
			const float armHWidth = 1.0f;

			auto& walls = Globals::Components().walls();

			windmillWall = walls.size();
			walls.emplace_back(Tools::CreateTrianglesBody({ 
				{ glm::vec2{0.0f, -armOverlap}, glm::vec2{armHWidth, armLength}, glm::vec2{-armHWidth, armLength} },
				{ glm::vec2{0.0f, armOverlap}, glm::vec2{-armHWidth, -armLength}, glm::vec2{armHWidth, -armLength} },
				{ glm::vec2{-armOverlap, 0.0f}, glm::vec2{armLength, armHWidth}, glm::vec2{armLength, -armHWidth} },
				{ glm::vec2{armOverlap, 0.0f}, glm::vec2{-armLength, -armHWidth}, glm::vec2{-armLength, armHWidth} } },
				Tools::BodyParams().bodyType(b2_kinematicBody))).renderLayer = RenderLayer::NearMidground;
			walls.back().texture = TCM::Texture(woodTexture);
		}

		void initHandlers()
		{
			/*Tools::UpdateNumOfPlayers(playersHandlers, rocketPlaneTexture, flameAnimatedTextureForPlayers, gamepadForPlayer1, true);

			missilesHandler.setPlayersHandlers(playersHandlers);
			missilesHandler.setExplosionTexture(explosionTexture);
			missilesHandler.setMissileTexture(missile2Texture);
			missilesHandler.setFlameAnimatedTexture(flameAnimatedTexture);
			missilesHandler.setResolutionModeF([this](const auto& targetBody) {
				return lowResBodies.count(&targetBody) ? ResolutionMode::LowPixelArtBlend1 : ResolutionMode::LowestLinearBlend1;
				});
			missilesHandler.setExplosionF([this]() {
				explosionFrame = true;
				});*/
		}

		void step()
		{
			{
				float mouseSensitivity = 0.01f;
				float gamepadSensitivity = 50.0f;

				const auto& physics = Globals::Components().physics();
				const auto& mouse = Globals::Components().mouse();
				const auto& gamepad = Globals::Components().gamepads()[0];
				auto& player1Controls = Globals::Components().planes()[player1Id].controls;

				player1Controls.turningDelta = mouse.getWorldSpaceDelta() * mouseSensitivity +
					Tools::ApplyDeadzone(gamepad.lStick) * physics.frameDuration * gamepadSensitivity;
				player1Controls.autoRotation = (bool)std::max((float)mouse.pressing.rmb, gamepad.rTrigger);
				player1Controls.throttling = std::max((float)mouse.pressing.rmb, gamepad.rTrigger);
				player1Controls.magneticHook = mouse.pressing.xmb1 || gamepad.pressing.lShoulder || gamepad.lTrigger >= 0.5f;
			}

			{
				const float rotationSpeed = 0.1f;

				const auto& physics = Globals::Components().physics();
				auto& windmill = Globals::Components().walls()[windmillWall];

				windmill.body->SetTransform(windmill.body->GetPosition(), windmill.body->GetAngle() + physics.frameDuration * rotationSpeed);
			}
		}

	private:
		unsigned rocketPlaneTexture = 0;
		unsigned woodTexture = 0;
		unsigned flame1AnimationTexture = 0;

		unsigned flame1AnimatedTexture = 0;

		unsigned windmillWall = 0;

		unsigned player1Id;

		Tools::MissilesHandler missilesHandler;
	};

	Windmill::Windmill():
		impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->setAnimations();
		impl->createPlayers();
		impl->setCamera();
		impl->createWindmill();
		impl->initHandlers();
	}

	Windmill::~Windmill() = default;

	void Windmill::step()
	{
		impl->step();
	}
}
