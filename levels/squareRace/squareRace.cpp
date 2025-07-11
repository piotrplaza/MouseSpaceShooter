#include "squareRace.hpp"

#include <components/graphicsSettings.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/camera2D.hpp>
#include <components/plane.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/polyline.hpp>
#include <components/soundBuffer.hpp>
#include <components/sound.hpp>
#include <components/music.hpp>
#include <components/collisionHandler.hpp>
#include <components/deferredAction.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <ogl/renderingHelpers.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/gameHelpers.hpp>
#include <tools/particleSystemHelpers.hpp>
#include <tools/playersHandler.hpp>
#include <tools/b2Helpers.hpp>
#include <tools/Shapes2D.hpp>

#include <glm/gtx/vector_angle.hpp>

#include <unordered_map>
#include <unordered_set>

namespace
{
	constexpr bool gamepadForPlayer1 = false;
	constexpr unsigned circuitsToEliminate = 3;
	constexpr float squareHSize = 100.0f;
	constexpr float ringRadius = 200.0f;
	constexpr float soundAttenuation = 0.02f;
}

namespace Levels
{
	class SquareRace::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			Globals::Components().graphicsSettings().backgroundColorF = glm::vec4{ 0.7f, 0.8f, 0.9f, 1.0f };
			Globals::Components().graphicsSettings().lineWidth = 5.0f;
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

			flameAnimationTexture = textures.size();
			textures.emplace("textures/flame animation 1.jpg");
			textures.last().minFilter = GL_LINEAR;

			explosionTexture = textures.size();
			textures.emplace("textures/explosion.png");

			cityTexture = textures.size();
			textures.emplace("textures/city.jpg");

			orbTexture = textures.size();
			textures.emplace("textures/orb.png");
			textures.last().scale = glm::vec2(2.0f);
		}

		void loadAudio()
		{
			auto& musics = Globals::Components().staticMusics();
			musics.emplace("audio/Ghosthack-Ambient Beds_Daylight_Am 75Bpm (WET).ogg", 1.0f).play();

			auto& soundsBuffers = Globals::Components().staticSoundsBuffers();
			thrustSoundBuffer = soundsBuffers.emplace("audio/thrust.wav", 0.4f).getComponentId();
			grappleSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Synth - Choatic_C.wav", 1.5f).getComponentId();
			collisionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Impact - Edge.wav", 1.5f).getComponentId();
			playerExplosionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack-AC21_Impact_Cracked.wav", 3.0f).getComponentId();
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

		void setCamera()
		{
			playersHandler.setCamera(Tools::PlayersHandler::CameraParams().projectionHSizeMin([]() { return 30.0f; }).scalingFactor(0.7f));
		}

		void createLevel()
		{
			{
				auto& staticWalls = Globals::Components().staticWalls();

				staticWalls.emplace(Tools::CreateBoxBody({ squareHSize, squareHSize }, Tools::BodyParams{}.friction(10.0f)), CM::Texture(cityTexture, true));
				staticWalls.last().texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();

				staticWalls.emplace(Tools::CreateDiscBody(1.0f, Tools::BodyParams().position({ ringRadius, 0.0f })), CM::Texture(orbTexture, true));
				staticWalls.emplace(Tools::CreateDiscBody(1.0f, Tools::BodyParams().position({ squareHSize, 0.0f })), CM::Texture(orbTexture, true));
			}

			{
				auto& staticPolylines = Globals::Components().staticPolylines();

				constexpr int numOfRingSegments = 200;
				constexpr float ringStep = glm::two_pi<float>() / numOfRingSegments;

				std::vector<glm::vec2> ringSegments;
				ringSegments.reserve(numOfRingSegments);
				for (int i = 0; i < numOfRingSegments; ++i)
				{
					ringSegments.emplace_back(glm::vec2(glm::cos(i * ringStep), glm::sin(i * ringStep)) * ringRadius);
				}
				ringSegments.push_back(ringSegments.front());
				auto& outerRing = staticPolylines.emplace(ringSegments, Tools::BodyParams().sensor(true));
				outerRing.segmentVerticesGenerator = [](const auto& v1, const auto& v2) { return Tools::Shapes2D::CreatePositionsOfLightning(v1, v2, 20, 0.2f); };
				outerRing.keyVerticesTransformer = [](std::vector<glm::vec3>& vertices) { Tools::VerticesDefaultRandomTranslate(vertices, true, 0.04f); };
				outerRing.colorF = [this]() {
					return (playersHandler.getActivePlayersHandlers().size() == 1
						? glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
						: glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)) * 0.8f;
				};

				auto& finishLine = staticPolylines.emplace(std::vector<glm::vec2>{ {squareHSize, 0.0f}, {ringRadius, 0.0f} }, Tools::BodyParams().sensor(true));
				finishLine.colorF = []() { return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); };
				finishStaticPolyline = finishLine.getComponentId();
			}
		}

		void destroyPlane(Components::Plane& plane)
		{
			Tools::CreateExplosion(Tools::ExplosionParams().center(plane.getOrigin2D()).sourceVelocity(plane.getVelocity()).numOfParticles(16).initExplosionVelocity(100.0f).particlesPerDecoration(1).
				initExplosionVelocityRandomMinFactor(0.2f).explosionTexture(CM::Texture(explosionTexture, true)));
			Tools::CreateAndPlaySound(CM::SoundBuffer(playerExplosionSoundBuffer, true), [pos = plane.getOrigin2D()]() { return pos; }).setAttenuation(soundAttenuation);
			plane.setEnabled(false);
			playersToCircuits.erase(plane.getComponentId());
		}

		void collisionHandlers()
		{
			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::actor, Globals::CollisionBits::polyline, [this](const auto& plane, const auto& polyline) {
				Globals::Components().deferredActions().emplace([&](auto) {
					auto& planeComponent = Tools::AccessComponent<CM::Plane>(plane);
					const auto& polylineComponent = Tools::AccessComponent<CM::Polyline>(polyline);

					if (polylineComponent.getComponentId() == finishStaticPolyline)
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

					if (polylineComponent.getComponentId() != finishStaticPolyline)
						return false;

					auto activePlayersHandlers = playersHandler.getActivePlayersHandlers();

					if (activePlayersHandlers.size() < 2)
						return false;

					if (planeComponent.getVelocity().y < 0.0f)
					{
						destroyPlane(planeComponent);
						return false;
					}

					if ((*collisionsStarted)[planeComponent.getComponentId()]++ > 0)
						return false;

					const glm::vec2 polylineVec = glm::normalize(polylineComponent.getPositions()[1] - polylineComponent.getPositions()[0]);
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

					if (polylineComponent.getComponentId() != finishStaticPolyline)
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
				Tools::CreateAndPlaySound(CM::SoundBuffer(collisionSoundBuffer, true),
					[pos = *Tools::GetCollisionPoint(*plane.GetBody(), *obstacle.GetBody())]() {
							return pos;
						},
						[&](auto& sound) {
							sound.setVolume(std::sqrt(Tools::GetRelativeSpeed(*plane.GetBody(), *obstacle.GetBody()) / 20.0f));
							sound.setPitch(Tools::RandomFloat(0.4f, 0.6f));
							sound.setAttenuation(soundAttenuation);
					});
				}));
		}

		void step()
		{
			playersHandler.controlStep();

			const auto& planes = Globals::Components().planes();
			const auto activePlayersHandlers = playersHandler.getActivePlayersHandlers();
			if (activePlayersHandlers.size() == 1 && planes[activePlayersHandlers.front()->playerId].controls.backPressed)
				reset();
		}

		void reset()
		{
			Globals::CleanupDynamicComponents();

			{
				auto& grapples = Globals::Components().grapples();

				constexpr float range = (ringRadius - squareHSize) * 0.6f;

				grapples.emplace(Tools::CreateDiscBody(1.0f, Tools::BodyParams().position({ -squareHSize, -squareHSize })), CM::Texture(orbTexture, true)).range = range;
				grapples.emplace(Tools::CreateDiscBody(1.0f, Tools::BodyParams().position({ squareHSize, -squareHSize })), CM::Texture(orbTexture, true)).range = range;
				grapples.emplace(Tools::CreateDiscBody(1.0f, Tools::BodyParams().position({ -squareHSize, squareHSize })), CM::Texture(orbTexture, true)).range = range;
				grapples.emplace(Tools::CreateDiscBody(1.0f, Tools::BodyParams().position({ squareHSize, squareHSize })), CM::Texture(orbTexture, true)).range = range;
			}

			playersHandler.initPlayers(Tools::PlayersHandler::InitPlayerParams{}.planeTextures(planeTextures).flameTextures(flameAnimatedTextureForPlayers).gamepadForPlayer1(gamepadForPlayer1).initLocationFunc(
				[this](unsigned playerId, unsigned numOfPlayers) {
					const float distance = 4.0f;
					return glm::vec3((squareHSize + ringRadius) * 0.5f - ((numOfPlayers - 1) * distance) * 0.5f + playerId * distance, -0.1f, glm::half_pi<float>());
				}).centerToFront(true).thrustSound(CM::SoundBuffer(thrustSoundBuffer, true)).grappleSound(CM::SoundBuffer(grappleSoundBuffer, true)).soundAttenuation(soundAttenuation));

			collisionHandlers();

			maxCircuits = 0;
			playersToCircuits.clear();
			auto activePlayersHandlers = playersHandler.getActivePlayersHandlers();
			for (const auto& activePlayerHandler : activePlayersHandlers)
				playersToCircuits[activePlayerHandler->playerId] = 0;
		}

	private:
		std::array<CM::Texture, 4> planeTextures;
		ComponentId flameAnimationTexture = 0;
		ComponentId explosionTexture = 0;
		ComponentId cityTexture = 0;
		ComponentId orbTexture = 0;

		std::array<CM::AnimatedTexture, 4> flameAnimatedTextureForPlayers;

		ComponentId playerExplosionSoundBuffer = 0;
		ComponentId thrustSoundBuffer = 0;
		ComponentId grappleSoundBuffer = 0;
		ComponentId collisionSoundBuffer = 0;
		ComponentId finishStaticPolyline = 0;

		Tools::PlayersHandler playersHandler;

		unsigned maxCircuits = 0;
		std::unordered_map<unsigned, unsigned> playersToCircuits;
	};

	SquareRace::SquareRace() :
		impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->loadAudio();
		impl->setAnimations();
		impl->setCamera();

		impl->createLevel();

		impl->reset();
	}

	SquareRace::~SquareRace() = default;

	void SquareRace::step()
	{
		impl->step();
	}
}
