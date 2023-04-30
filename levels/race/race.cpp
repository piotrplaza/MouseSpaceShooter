#include "race.hpp"

#include <components/graphicsSettings.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/camera.hpp>
#include <components/plane.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>
#include <components/soundBuffer.hpp>
#include <components/sound.hpp>
#include <components/polyline.hpp>
#include <components/collisionHandler.hpp>
#include <components/deferredAction.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <ogl/renderingHelpers.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/gameHelpers.hpp>
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
			Globals::Components().graphicsSettings().clearColor = { 0.7f, 0.8f, 0.9f, 1.0f };
			Globals::Components().mainFramebufferRenderer().renderer = Tools::StandardFullscreenRenderer(Globals::Shaders().textured());
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

			explosionTexture = textures.size();
			textures.emplace("textures/explosion.png");
		}

		void loadAudio()
		{
			auto& soundsBuffers = Globals::Components().soundsBuffers();

			thrustSoundBuffer = soundsBuffers.emplace("audio/thrust.wav", 0.2f).getComponentId();
			grappleSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Synth - Choatic_C.wav").getComponentId();
			playerExplosionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack-AC21_Impact_Cracked.wav").getComponentId();
			collisionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Impact - Edge.wav").getComponentId();
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

		void setCamera()
		{
			playersHandler.setCamera(Tools::PlayersHandler::CameraParams().projectionHSizeMin([]() { return 30.0f; }).scalingFactor(0.7f));
		}

		void generatedCode()
		{
			GeneratedCode::CreateStartingLine(startingStaticPolyline);
			GeneratedCode::CreateDeadlySplines(playersHandler, deadlySplines);
		}

		void collisions()
		{
			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::plane, Globals::CollisionBits::polyline,
			[this](const auto& plane, const auto& polyline) {
				Globals::Components().deferredActions().emplace([&](auto) {
					auto& planeComponent = Tools::AccessComponent<TCM::Plane>(plane);
					const auto& polylineComponent = Tools::AccessComponent<TCM::StaticPolyline>(polyline);

					if (!deadlySplines.contains(polylineComponent.getComponentId()))
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

			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::plane, Globals::CollisionBits::polyline, [this, collisionsStarted, collisionsBlocked](const auto& plane, const auto& polyline) {
				Globals::Components().deferredActions().emplace([&, collisionsStarted, collisionsBlocked](auto) {
					auto& planeComponent = Tools::AccessComponent<TCM::Plane>(plane);
					const auto& polylineComponent = Tools::AccessComponent<TCM::StaticPolyline>(polyline);

					if (polylineComponent.getComponentId() != startingStaticPolyline)
						return false;

					auto activePlayersHandlers = playersHandler.getActivePlayersHandlers();

					if (activePlayersHandlers.size() < 2)
						return false;

					const glm::vec2 polylineVec = glm::normalize(polylineComponent.getVertices()[1] - polylineComponent.getVertices()[0]);
					const glm::vec2 planeVelocityVec = glm::normalize(planeComponent.getVelocity());

					if (glm::orientedAngle(polylineVec, planeVelocityVec) > 0.0f)
					{
						destroyPlane(planeComponent);
						return false;
					}

					if ((*collisionsStarted)[planeComponent.getComponentId()]++ > 0)
						return false;

					const glm::vec2 planePrevVec = glm::normalize(planeComponent.details.previousCenter - glm::vec2(polylineComponent.getVertices()[0]));

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

			Globals::Components().endCollisionHandlers().emplace(Globals::CollisionBits::plane, Globals::CollisionBits::polyline, [this, collisionsStarted, collisionsBlocked](const auto& plane, auto& polyline) {
				Globals::Components().deferredActions().emplace([&, collisionsStarted, collisionsBlocked](auto) {
					const auto& planeComponent = Tools::AccessComponent<TCM::Plane>(plane);
					const auto& polylineComponent = Tools::AccessComponent<TCM::StaticPolyline>(polyline);

					if (polylineComponent.getComponentId() != startingStaticPolyline)
						return false;

					const glm::vec2 polylineVec = glm::normalize(polylineComponent.getVertices()[1] - polylineComponent.getVertices()[0]);
					const glm::vec2 planeVec = glm::normalize(planeComponent.getCenter() - glm::vec2(polylineComponent.getVertices()[0]));

					if (--(*collisionsStarted)[planeComponent.getComponentId()] > 0)
						return false;

					if (glm::orientedAngle(polylineVec, planeVec) > 0.0f)
						return false;

					collisionsBlocked->erase(planeComponent.getComponentId());

					return false;
					});
				});

			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::plane, Globals::CollisionBits::plane | Globals::CollisionBits::wall,
				Tools::SkipDuplicatedBodiesCollisions([this](const auto& plane, const auto& obstacle) {
					Tools::PlaySingleSound(collisionSoundBuffer,
					[pos = *Tools::GetCollisionPoint(*plane.GetBody(), *obstacle.GetBody())]() {
							return pos;
						},
						[&](auto& sound) {
							sound.volume(std::sqrt(Tools::GetRelativeVelocity(*plane.GetBody(), *obstacle.GetBody()) / 20.0f));
						});
					}));
		}

		void step()
		{
			playersHandler.controlStep([this](unsigned playerHandlerId, bool fire) {
				if (playersHandler.getActivePlayersHandlers().size() == 1 && Globals::Components().planes()[playersHandler.getActivePlayersHandlers().front()->playerId].controls.startPressed)
					reset();
			});
		}

		void destroyPlane(Components::Plane& plane)
		{
			Tools::CreateExplosion(Tools::ExplosionParams().center(plane.getCenter()).sourceVelocity(plane.getVelocity()).
				initExplosionVelocityRandomMinFactor(0.2f).explosionTexture(explosionTexture));
			Tools::PlaySingleSound(playerExplosionSoundBuffer, [pos = plane.getCenter()]() { return pos; });
			plane.enable(false);
			playersToCircuits.erase(plane.getComponentId());
		}

		void reset()
		{
			Globals::MarkDynamicComponentsAsDirty();

			playersHandler.initPlayers(planeTextures, flameAnimatedTextureForPlayers, false,
				[this](auto, auto) {
					return glm::vec3(0.0f);
				}, true, thrustSoundBuffer, grappleSoundBuffer);

			collisions();

			maxCircuits = 0;
			playersToCircuits.clear();
			auto activePlayersHandlers = playersHandler.getActivePlayersHandlers();
			for (const auto& activePlayerHandler : activePlayersHandlers)
				playersToCircuits[activePlayerHandler->playerId] = 0;
		}

	private:
		std::array<unsigned, 4> planeTextures{ 0 };
		ComponentId flameAnimationTexture = 0;
		ComponentId explosionTexture = 0;

		std::array<unsigned, 4> flameAnimatedTextureForPlayers{ 0 };

		ComponentId thrustSoundBuffer = 0;
		ComponentId grappleSoundBuffer = 0;
		ComponentId playerExplosionSoundBuffer = 0;
		ComponentId collisionSoundBuffer = 0;

		ComponentId startingStaticPolyline = 0;

		Tools::PlayersHandler playersHandler;

		std::unordered_set<ComponentId> deadlySplines;
		unsigned maxCircuits = 0;
		std::unordered_map<unsigned, unsigned> playersToCircuits;
	};

	Race::Race() :
		impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->loadAudio();
		impl->setAnimations();
		impl->setCamera();
		impl->generatedCode();
		impl->collisions();
		impl->reset();
	}

	Race::~Race() = default;

	void Race::step()
	{
		impl->step();
	}
}
