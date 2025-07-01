#include "race.hpp"

#include <components/graphicsSettings.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/camera2D.hpp>
#include <components/plane.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>
#include <components/soundBuffer.hpp>
#include <components/sound.hpp>
#include <components/music.hpp>
#include <components/polyline.hpp>
#include <components/collisionHandler.hpp>
#include <components/deferredAction.hpp>
#include <components/grapple.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <ogl/renderingHelpers.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/gameHelpers.hpp>
#include <tools/particleSystemHelpers.hpp>
#include <tools/playersHandler.hpp>
#include <tools/splines.hpp>

#include "generatedCode.hpp"

#include <glm/gtx/vector_angle.hpp>

namespace
{
	unsigned circuitsToEliminate = 1;
}

namespace Levels
{
	class Race::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			auto& graphicsSettings = Globals::Components().graphicsSettings();
			graphicsSettings.cullFace = false;
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

			flameAnimationTextureId = textures.size();
			textures.emplace("textures/flame animation 1_1.jpg");
			textures.last().minFilter = GL_LINEAR;

			explosionTextureId = textures.size();
			textures.emplace("textures/explosion.png");
		}

		void loadAudio()
		{
			auto& musics = Globals::Components().staticMusics();
			musics.emplace(GeneratedCode::musicPath, 1.0f).play();

			auto& soundsBuffers = Globals::Components().staticSoundsBuffers();
			thrustSoundBufferId = soundsBuffers.emplace("audio/thrust.wav", 0.2f).getComponentId();
			grappleSoundBufferId = soundsBuffers.emplace("audio/Ghosthack Synth - Choatic_C.wav").getComponentId();
			playerExplosionSoundBufferId = soundsBuffers.emplace("audio/Ghosthack-AC21_Impact_Cracked.wav").getComponentId();
			collisionSoundBufferId = soundsBuffers.emplace("audio/Ghosthack Impact - Edge.wav").getComponentId();
		}

		void setAnimations()
		{
			for (auto& flameAnimatedTextureForPlayer : flameAnimatedTextureForPlayers)
			{
				flameAnimatedTextureForPlayer = Globals::Components().staticAnimatedTextures().add({ CM::Texture(flameAnimationTextureId, true), { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 115 }, 0.02f, 32, 0,
					AnimationData::Direction::Backward, AnimationData::Mode::Repeat, AnimationData::TextureLayout::Horizontal });
				Globals::Components().staticAnimatedTextures().last().start(true);
			}
		}

		void setCamera()
		{
			const glm::vec2 levelHSize = GeneratedCode::backgroundImageScale * 0.5f * glm::vec2(GeneratedCode::backgroundImageAspectRatio, 1.0f);
			playersHandler.setCamera(Tools::PlayersHandler::CameraParams().projectionHSizeMin(GeneratedCode::projectionHSizeMin).transitionFactor(2.0f).scalingFactor(0.9f).velocityFactor(1.0f)
				.boundaryParams_levelHSize_trackingMargin({ levelHSize, 0.0f }));
		}

		void generatedElements()
		{
			auto& graphicsSettings = Globals::Components().graphicsSettings();
			graphicsSettings.backgroundColorF = GeneratedCode::backgroundColor;
			GeneratedCode::CreateBackground(backgroundTextureId, backgroundDecorationId);
			GeneratedCode::CreateStartingLine(startingStaticPolylineId, startingLineP1, startingLineP2, startingPositionLineDistance);
			GeneratedCode::CreateDeadlySplines(playersHandler, deadlySplineIds);
			GeneratedCode::CreateGrapples();
		}

		void customElements()
		{
			const glm::vec2 levelHSize = GeneratedCode::backgroundImageScale * 0.5f * glm::vec2(GeneratedCode::backgroundImageAspectRatio, 1.0f);
			auto& walls = Globals::Components().staticWalls();

			{
				const float wallHWidth = 2.0f;
				walls.emplace(Tools::CreateBoxBody({ wallHWidth, levelHSize.y + wallHWidth * 2.0f }, Tools::BodyParams().position({ -levelHSize.x - wallHWidth, 0.0f })));
				walls.emplace(Tools::CreateBoxBody({ wallHWidth, levelHSize.y + wallHWidth * 2.0f }, Tools::BodyParams().position({ levelHSize.x + wallHWidth, 0.0f })));
				walls.emplace(Tools::CreateBoxBody({ levelHSize.x + wallHWidth * 2.0f, wallHWidth }, Tools::BodyParams().position({ 0.0f, -levelHSize.y - wallHWidth })));
				walls.emplace(Tools::CreateBoxBody({ levelHSize.x + wallHWidth * 2.0f, wallHWidth }, Tools::BodyParams().position({ 0.0f, levelHSize.y + wallHWidth })));
			}
		}

		void initCollisions()
		{
			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::actor, Globals::CollisionBits::polyline,
			[this](const auto& plane, const auto& polyline) {
				Globals::Components().deferredActions().emplace([&](auto) {
					auto& planeComponent = Tools::AccessComponent<CM::Plane>(plane);
					const auto& polylineComponent = Tools::AccessComponent<CM::Polyline>(polyline);

					if (!deadlySplineIds.contains(polylineComponent.getComponentId()))
						return false;

					auto activePlayersHandlers = playersHandler.getActivePlayersHandlers();

					if (activePlayersHandlers.size() < 2)
						return false;

					destroyPlane(planeComponent);

					return false;
				});
			});

			auto collisionsStarted = std::make_shared<std::unordered_map<ComponentId, int>>();
			auto collisionsBlocked = std::make_shared<std::unordered_set<ComponentId>>();

			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::actor, Globals::CollisionBits::polyline, [this, collisionsStarted, collisionsBlocked](const auto& plane, const auto& polyline) {
				Globals::Components().deferredActions().emplace([&, collisionsStarted, collisionsBlocked](auto) {
					auto& planeComponent = Tools::AccessComponent<CM::Plane>(plane);
					const auto& polylineComponent = Tools::AccessComponent<CM::Polyline>(polyline);

					if (polylineComponent.getComponentId() != startingStaticPolylineId)
						return false;

					auto activePlayersHandlers = playersHandler.getActivePlayersHandlers();

					if (activePlayersHandlers.size() < 2)
						return false;

					const glm::vec2 polylineVec = glm::normalize(polylineComponent.getPositions()[1] - polylineComponent.getPositions()[0]);
					const glm::vec2 planeVelocityVec = glm::normalize(planeComponent.getVelocity());

					if (glm::orientedAngle(polylineVec, planeVelocityVec) > 0.0f)
					{
						destroyPlane(planeComponent);
						return false;
					}

					if ((*collisionsStarted)[planeComponent.getComponentId()]++ > 0)
						return false;

					const glm::vec2 planePrevVec = glm::normalize(planeComponent.details.previousCenter - glm::vec2(polylineComponent.getPositions()[0]));

					if (glm::orientedAngle(polylineVec, planePrevVec) < 0.0f)
						return false;

					if (!collisionsBlocked->insert(planeComponent.getComponentId()).second)
						return false;

					const unsigned newCircuits = ++playersToCircuits[planeComponent.getComponentId()];
					maxCircuits = std::max(maxCircuits, newCircuits);

					if (maxCircuits > circuitsToEliminate)
					{
						unsigned numOfWorsePlayers = 0;
						unsigned worsePlayerId = 0;

						for (auto* activePlayerHandler : activePlayersHandlers)
						{
							if (activePlayerHandler->playerId == planeComponent.getComponentId())
								continue;

							if (playersToCircuits.at(activePlayerHandler->playerId) < maxCircuits)
							{
								++numOfWorsePlayers;
								worsePlayerId = activePlayerHandler->playerId;
							}
						}

						if (numOfWorsePlayers == 1)
							destroyPlane(Globals::Components().planes()[worsePlayerId]);
					}
					return false;
				});
			});

			Globals::Components().endCollisionHandlers().emplace(Globals::CollisionBits::actor, Globals::CollisionBits::polyline, [this, collisionsStarted, collisionsBlocked](const auto& plane, auto& polyline) {
				Globals::Components().deferredActions().emplace([&, collisionsStarted, collisionsBlocked](auto) {
					const auto& planeComponent = Tools::AccessComponent<CM::Plane>(plane);
					const auto& polylineComponent = Tools::AccessComponent<CM::Polyline>(polyline);

					if (polylineComponent.getComponentId() != startingStaticPolylineId)
						return false;

					const glm::vec2 polylineVec = glm::normalize(polylineComponent.getPositions()[1] - polylineComponent.getPositions()[0]);
					const glm::vec2 planeVec = glm::normalize(planeComponent.getOrigin2D() - glm::vec2(polylineComponent.getPositions()[0]));

					if (--(*collisionsStarted)[planeComponent.getComponentId()] > 0)
						return false;

					if (glm::orientedAngle(polylineVec, planeVec) > 0.0f)
						return false;

					collisionsBlocked->erase(planeComponent.getComponentId());

					return false;
				});
			});

			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::actor, Globals::CollisionBits::actor | Globals::CollisionBits::wall,
				Tools::SkipDuplicatedBodiesCollisions([this](const auto& plane, const auto& obstacle) {
					Tools::CreateAndPlaySound(CM::SoundBuffer(collisionSoundBufferId, true),
					[pos = *Tools::GetCollisionPoint(*plane.GetBody(), *obstacle.GetBody())]() {
							return pos;
						},
						[&](auto& sound) {
							sound.setVolume(std::sqrt(Tools::GetRelativeSpeed(*plane.GetBody(), *obstacle.GetBody()) / 20.0f));
						});
					}
				)
			);
		}

		void step()
		{
			playersHandler.controlStep();

			const auto& planes = Globals::Components().planes();
			const auto activePlayersHandlers = playersHandler.getActivePlayersHandlers();
			if (activePlayersHandlers.size() == 1 && planes[activePlayersHandlers.front()->playerId].controls.backPressed)
				reset();
		}

		void destroyPlane(Components::Plane& plane)
		{
			Tools::CreateExplosion(Tools::ExplosionParams().center(plane.getOrigin2D()).sourceVelocity(plane.getVelocity()).
				initExplosionVelocityRandomMinFactor(0.2f).explosionTexture(CM::Texture(explosionTextureId, true)));
			Tools::CreateAndPlaySound(CM::SoundBuffer(playerExplosionSoundBufferId, true), [pos = plane.getOrigin2D()]() { return pos; });
			plane.setEnabled(false);
			playersToCircuits.erase(plane.getComponentId());
		}

		void reset()
		{
			Globals::CleanupDynamicComponents();

			customElements();

			playersHandler.initPlayers(Tools::PlayersHandler::InitPlayerParams{}.planeTextures(planeTextures).flameTextures(flameAnimatedTextureForPlayers).gamepadForPlayer1(false).initLocationFunc(
				[&](unsigned playerId, unsigned numOfPlayers) {
					if (glm::distance(startingLineP1, startingLineP2) == 0.0f)
						return glm::vec3(0.0f, 0.0f, 0.0f);

					const glm::vec2 startingLineEndsVector = startingLineP2 - startingLineP1;
					const float startingLineLength = glm::length(startingLineEndsVector);
					const float playerPositionOnStartingLine = startingLineLength * (playerId + 1) / (numOfPlayers + 1);
					const glm::vec2 playerPositionOnStartingLine2D = startingLineP1 + startingLineEndsVector * playerPositionOnStartingLine / startingLineLength;
					const glm::vec2 ntv = glm::rotate(glm::normalize(startingLineEndsVector), -glm::half_pi<float>());

					return glm::vec3(playerPositionOnStartingLine2D + ntv * startingPositionLineDistance,
						glm::orientedAngle({ -1.0f, 0.0f }, ntv));
				}).centerToFront(true).thrustSound(CM::SoundBuffer(thrustSoundBufferId, true)).grappleSound(CM::SoundBuffer(grappleSoundBufferId, true)).soundAttenuation(0.1f));

			initCollisions();

			maxCircuits = 0;
			playersToCircuits.clear();
			auto activePlayersHandlers = playersHandler.getActivePlayersHandlers();
			for (const auto& activePlayerHandler : activePlayersHandlers)
				playersToCircuits[activePlayerHandler->playerId] = 0;
		}

	private:
		std::array<CM::Texture, 4> planeTextures;
		ComponentId flameAnimationTextureId = 0;
		ComponentId explosionTextureId = 0;

		std::array<CM::AnimatedTexture, 4> flameAnimatedTextureForPlayers;

		ComponentId thrustSoundBufferId = 0;
		ComponentId grappleSoundBufferId = 0;
		ComponentId playerExplosionSoundBufferId = 0;
		ComponentId collisionSoundBufferId = 0;
		ComponentId backgroundTextureId = 0;
		ComponentId backgroundDecorationId = 0;
		ComponentId startingStaticPolylineId = 0;

		Tools::PlayersHandler playersHandler;

		std::unordered_set<ComponentId> deadlySplineIds;
		unsigned maxCircuits = 0;
		std::unordered_map<unsigned, unsigned> playersToCircuits;

		glm::vec2 startingLineP1{ 0.0f };
		glm::vec2 startingLineP2{ 0.0f };
		float startingPositionLineDistance = 0.0f;
	};

	Race::Race() :
		impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->loadAudio();
		impl->setAnimations();
		impl->setCamera();
		impl->generatedElements();
		impl->reset();
	}

	Race::~Race() = default;

	void Race::step()
	{
		impl->step();
	}
}
