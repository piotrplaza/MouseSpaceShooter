#include "squareRace.hpp"

#include <components/graphicsSettings.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/camera.hpp>
#include <components/plane.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>
#include <components/staticWall.hpp>
#include <components/staticPolyline.hpp>
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
#include <tools/playersHandler.hpp>
#include <tools/b2Helpers.hpp>

namespace Levels
{
	class SquareRace::Impl
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
			auto& musics = Globals::Components().musics();
			musics.emplace("audio/Ghosthack-Ambient Beds_Daylight_Am 75Bpm (WET).ogg", 1.0f).play();

			auto& soundsBuffers = Globals::Components().soundsBuffers();
			thrustSoundBuffer = soundsBuffers.emplace("audio/thrust.wav", 0.2f).getComponentId();
			grappleSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Synth - Choatic_C.wav").getComponentId();
			collisionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Impact - Edge.wav").getComponentId();
			playerExplosionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack-AC21_Impact_Cracked.wav").getComponentId();
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

		void createLevel()
		{
			{
				auto& staticWalls = Globals::Components().staticWalls();

				auto& squareWall = staticWalls.emplace(Tools::CreateBoxBody({ 100.0f, 100.0f }), TCM::Texture(cityTexture));
				squareWall.texCoord = Tools::CreateTexCoordOfRectangle();

				staticWalls.emplace(Tools::CreateCircleBody(1.0f, Tools::BodyParams().position({160.0f, 0.0f})), TCM::Texture(orbTexture));
				staticWalls.emplace(Tools::CreateCircleBody(1.0f, Tools::BodyParams().position({ 100.0f, 0.0f })), TCM::Texture(orbTexture));
			}

			{
				auto& staticPolylines = Globals::Components().staticPolylines();

				constexpr float outerRingR = 160.0f;
				constexpr int numOfRingSegments = 200;
				constexpr float ringStep = glm::two_pi<float>() / numOfRingSegments;

				std::vector<glm::vec2> ringSegments;
				ringSegments.reserve(numOfRingSegments);
				for (int i = 0; i < numOfRingSegments; ++i)
				{
					ringSegments.emplace_back(glm::vec2(glm::cos(i * ringStep), glm::sin(i * ringStep)) * outerRingR);
				}
				ringSegments.push_back(ringSegments.front());
				auto& outerRing = staticPolylines.emplace(ringSegments, Tools::BodyParams().sensor(true));
				outerRing.segmentVerticesGenerator = [](const auto& v1, const auto& v2) {
					return Tools::CreateVerticesOfLightning(v1, v2, 20, 0.2f);
				};
				outerRing.keyVerticesTransformer = [rD = 0.2f](const auto& v) {
					return v + glm::vec3(Tools::Random(-rD, rD), Tools::Random(-rD, rD), 0.0f);
				};
				outerRing.loop = true;
				outerRing.colorF = [this]() {
					return (playersHandler.getActivePlayersHandlers().size() == 1
						? glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
						: glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)) * 0.8f;
				};

				auto& finishLine = staticPolylines.emplace(std::vector<glm::vec2>{ {100.0f, 0.0f}, {160.0f, 0.0f} }, Tools::BodyParams().sensor(true));
				finishLine.colorF = []() { return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); };
				finishStaticPolyline = finishLine.getComponentId();
			}
		}

		void destroyPlane(Components::Plane& plane)
		{
			Tools::CreateExplosion(Tools::ExplosionParams().center(plane.getCenter()).sourceVelocity(plane.getVelocity()).
				initExplosionVelocityRandomMinFactor(0.2f).explosionTexture(explosionTexture));
			Tools::PlaySingleSound(playerExplosionSoundBuffer, [pos = plane.getCenter()]() { return pos; });
			plane.enable(false);
			playersToCircuits.erase(plane.getComponentId());
		}

		void collisionHandlers()
		{
			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::plane, Globals::CollisionBits::polyline, [this](const auto& plane, const auto& polyline) {
				Globals::Components().deferredActions().emplace([&](auto) {
					auto activePlayersHandlers = playersHandler.getActivePlayersHandlers();

					if (activePlayersHandlers.size() == 1)
						return false;

					auto& planeComponent = Tools::AccessComponent<TCM::Plane>(plane);
					const auto& polylineComponent = Tools::AccessComponent<TCM::StaticPolyline>(polyline);

					if (polylineComponent.getComponentId() != finishStaticPolyline || planeComponent.getVelocity().y < 0.0f)
					{
						destroyPlane(planeComponent);
						return false;
					}

					const unsigned newCircuits = ++playersToCircuits[planeComponent.getComponentId()];
					maxCircuits = std::max(maxCircuits, newCircuits);

					if (maxCircuits > 0)
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
			playersHandler.controlStep();

			auto& planes = Globals::Components().planes();
			auto activePlayersHandlers = playersHandler.getActivePlayersHandlers();
			if (activePlayersHandlers.size() == 1 && planes[activePlayersHandlers.front()->playerId].controls.startPressed)
				reset();
		}

		void reset()
		{
			Globals::MarkDynamicComponentsAsDirty();

			playersHandler.initPlayers(planeTextures, flameAnimatedTextureForPlayers, false,
				[this](unsigned player, auto) {
					return glm::vec3(110.0f + player * 5.0f, 3.0f, glm::half_pi<float>());
				}, thrustSoundBuffer, grappleSoundBuffer);

			collisionHandlers();

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
		ComponentId cityTexture = 0;
		ComponentId orbTexture = 0;

		std::array<unsigned, 4> flameAnimatedTextureForPlayers{ 0 };

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
