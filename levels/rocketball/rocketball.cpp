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
		}

		void setBackground() const
		{
			using namespace Globals::Components;

			backgroundDecorations.emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 100.0f, 60.0f }), playFieldTexture);
			backgroundDecorations.back().texCoord = Tools::CreateTexCoordOfRectangle();
		}

		void setPlayers()
		{
			using namespace Globals::Components;

			player1 = &players.emplace_back(Tools::CreateTrianglePlayerBody(2.0f, 0.2f), rocketPlaneTexture);
			player1->setPosition({ -10.0f, 0.0f });
			player1->connectIfApproaching = true;
			player1->autoRotationFactor = 0.0f;
			player1->renderingSetup = [
				colorUniform = Uniforms::UniformController4f()
			](Shaders::ProgramId program) mutable {
					if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
					const float fade = (glm::sin(Globals::Components::physics.simulationTime * 2.0f * glm::two_pi<float>()) + 1.0f) / 2.0f;
					colorUniform.setValue({ fade, 1.0f, fade, 1.0f });

					return nullptr;
				};

				for (int i = 0; i < 2; ++i)
				{
					auto& player1Thrust = player1Thrusts[i];

					backgroundDecorations.emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, -0.45f }, { 0.5f, 0.5f }), flameAnimation1Texture);
					backgroundDecorations.back().renderingSetup = [&, i,
						modelUniform = Uniforms::UniformControllerMat4f(),
						thrustScale = 1.0f
					](Shaders::ProgramId program) mutable {
						if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
						modelUniform.setValue(glm::scale(glm::rotate(glm::translate(Tools::GetModelMatrix(*player1->body),
							{ -0.9f, i == 0 ? -0.42f : 0.42f, 0.0f }),
							-glm::half_pi<float>() + (i == 0 ? 0.1f : -0.1f), { 0.0f, 0.0f, 1.0f }),
							{ std::min(thrustScale * 0.5f, 0.7f), thrustScale, 1.0f }));

						const float targetFrameTimeFactor = Globals::Components::physics.targetFrameTimeFactor;
						if (player1->throttling) thrustScale = std::min(thrustScale * (1.0f + targetFrameTimeFactor * 0.1f), 5.0f);
						else thrustScale = 1.0f + (thrustScale - 1.0f) * (1.0f - targetFrameTimeFactor * 0.1f);

						glBlendFunc(GL_ONE, GL_ONE);

						return []() { glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); };
					};

					backgroundDecorations.back().animationController.reset(new Tools::TextureAnimationController(
						{ 500, 498 }, { 2, 0 }, { 61, 120 }, { 8, 4 }, { 62.5f, 124.9f }, 0.02f, 0,
						AnimationLayout::Horizontal, AnimationPlayback::Backward, AnimationPolicy::Repeat,
						{ 0.0f, -0.45f }, { 1.0f, 1.0f }));
					player1Thrust = backgroundDecorations.back().animationController.get();
					player1Thrust->start();
				}
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

			camera.targetProjectionHSizeF = [&]() {
				camera.projectionTransitionFactor = 0.1f * physics.targetFrameTimeFactor;
				return 30.0f + glm::distance(player1->getCenter(), ball->getCenter()) * 0.3f;
			};
			camera.targetPositionF = [&]() {
				camera.positionTransitionFactor = 0.1f * physics.targetFrameTimeFactor;
				return (player1->getCenter() + ball->getCenter()) * 0.5f;
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

		Components::Player* player1 = nullptr;
		Components::Grapple* ball = nullptr;
		Tools::TextureAnimationController* player1Thrusts[2] = {};
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
