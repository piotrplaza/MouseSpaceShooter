#include "windmill.hpp"

#include <components/graphicsSettings.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/camera.hpp>
#include <components/plane.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>
#include <components/staticWall.hpp>
#include <components/grapple.hpp>
#include <components/polyline.hpp>
#include <components/decoration.hpp>
#include <components/renderingSetup.hpp>
#include <components/physics.hpp>
#include <components/collisionHandler.hpp>
#include <components/deferredAction.hpp>
#include <components/music.hpp>
#include <components/soundBuffer.hpp>
#include <components/sound.hpp>
#include <components/missile.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <ogl/renderingHelpers.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/gameHelpers.hpp>
#include <tools/b2Helpers.hpp>
#include <tools/graphicsHelpers.hpp>
#include <tools/b2Helpers.hpp>
#include <tools/playersHandler.hpp>
#include <tools/missilesHandler.hpp>

#include <numeric>

namespace Levels
{
	class Windmill::Impl
	{
	public:
		void setGraphicsSettings()
		{
			Globals::Components().graphicsSettings().defaultColor = { 0.7f, 0.7f, 0.7f, 1.0f };
			Globals::Components().mainFramebufferRenderer().renderer = Tools::StandardFullscreenRenderer(Globals::Shaders().textured());

			recursiveFaceRS = Globals::Components().renderingSetups().size();
			Globals::Components().renderingSetups().emplace([
				this,
				visibilityReduction = Uniforms::Uniform1b(),
				visibilityCenter = Uniforms::Uniform2f(),
				fullVisibilityDistance = Uniforms::Uniform1f(),
				invisibilityDistance = Uniforms::Uniform1f()
			](Shaders::ProgramId program) mutable {
				if (!visibilityReduction.isValid())
				{
					visibilityReduction = Uniforms::Uniform1b(program, "visibilityReduction");
					visibilityCenter = Uniforms::Uniform2f(program, "visibilityCenter");
					fullVisibilityDistance = Uniforms::Uniform1f(program, "fullVisibilityDistance");
					invisibilityDistance = Uniforms::Uniform1f(program, "invisibilityDistance");
				}

				visibilityReduction(true);
				visibilityCenter({ 0.0f, 0.0f });
				fullVisibilityDistance(0.0f);
				invisibilityDistance(12.0f * innerForceScale);

				return [=]() mutable {
					visibilityReduction(false);
				};
			});
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().textures();

			rocketPlaneTexture = textures.size();
			textures.emplace("textures/rocket plane.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.6f, 1.8f);

			woodTexture = textures.size();
			textures.emplace("textures/wood.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(20.0f);

			orbTexture = textures.size();
			textures.emplace("textures/orb.png");
			textures.last().scale = glm::vec2(2.0f);

			explosionTexture = textures.size();
			textures.emplace("textures/explosion.png");

			missileTexture = textures.size();
			textures.emplace("textures/missile2.png");
			textures.last().minFilter = GL_LINEAR;
			textures.last().scale = glm::vec2(0.4f, 0.45f);

			flameAnimationTexture = textures.size();
			textures.emplace("textures/flame animation 1.jpg");
			textures.last().minFilter = GL_LINEAR;

			recursiveFaceAnimationTexture = textures.size();
			textures.emplace("textures/recursive face animation.jpg");
			textures.last().minFilter = GL_LINEAR;
		}

		void loadAudio()
		{
			auto& musics = Globals::Components().musics();
			musics.emplace("audio/Ghosthack-Ambient Beds_Darkest Hour_Am 70Bpm (WET).ogg", 0.8f).play();

			auto& soundsBuffers = Globals::Components().soundsBuffers();
			missileExplosionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Impact - Detonate.wav").getComponentId();
			playerExplosionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack-AC21_Impact_Cracked.wav").getComponentId();
			missileLaunchingSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Whoosh - 5.wav", 0.2f).getComponentId();
			collisionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Impact - Edge.wav").getComponentId();
			thrustSoundBuffer = soundsBuffers.emplace("audio/thrust.wav", 0.2f).getComponentId();
			grappleSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Synth - Choatic_C.wav").getComponentId();
			innerForceSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Scrape - Horror_C.wav", 0.8f).getComponentId();
			
			innerForceSound = Tools::PlaySingleSound(innerForceSoundBuffer, []() { return glm::vec2(0.0f); },
				[](auto& sound) {
					sound.loop(true);
					sound.volume(0.0f);
				}).getComponentId();
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

			flameAnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().add(Globals::Components().animatedTextures().last());

			recursiveFaceAnimatedTexture = Globals::Components().animatedTextures().size();
			Globals::Components().animatedTextures().add({ recursiveFaceAnimationTexture, { 263, 525 }, { 5, 10 }, { 0, 0 }, 210, 473, { 52, 52 }, 0.02f, 50, 0,
				AnimationDirection::Forward, AnimationPolicy::Repeat, TextureLayout::Horizontal });
			Globals::Components().animatedTextures().last().start(true);
		}

		void createBackground() const
		{
			Tools::CreateJuliaBackground([this]() {
				const auto averageCenter = std::accumulate(playersHandler.getPlayersHandlers().begin(), playersHandler.getPlayersHandlers().end(),
					glm::vec2(0.0f), [](const auto& acc, const auto& currentHandler) {
						return acc + Globals::Components().planes()[currentHandler.playerId].getCenter();
					}) / (float)playersHandler.getPlayersHandlers().size();
					return averageCenter * 0.0001f; });
		}

		void createForeground() const
		{
			auto& staticDecorations = Globals::Components().staticDecorations();
			const auto& physics = Globals::Components().physics();

			staticDecorations.emplace(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 15.0f, 15.0f }),
				TCM::AnimatedTexture(recursiveFaceAnimatedTexture), Tools::CreateTexCoordOfRectangle(), recursiveFaceRS, RenderLayer::NearForeground);
			staticDecorations.last().modelMatrixF = [&, angle = 0.0f]() mutable {
				return glm::rotate(glm::scale(glm::mat4(1.0f), glm::vec3(innerForceScale)), angle += 2.0f * physics.frameDuration, { 0.0f, 0.0f, 1.0f });
			};
			staticDecorations.last().colorF = []() {
				return glm::vec4(0.4f);
			};
		}

		void setCamera() const
		{
			playersHandler.initMultiplayerCamera([]() { return 30.0f; }, 0.7f);
		}

		void createWindmill()
		{
			constexpr float armLength = 30.0f;
			constexpr float armOverlap = 5.0f;
			constexpr float armHWidth = 10.0f;

			constexpr float grappleR = armLength + 5.0f;

			auto& staticWalls = Globals::Components().staticWalls();
			const auto& physics = Globals::Components().physics();

			windmillWall = staticWalls.size();
			staticWalls.emplace(Tools::CreateTrianglesBody({ 
				{ glm::vec2{0.0f, -armOverlap}, glm::vec2{armHWidth, armLength}, glm::vec2{-armHWidth, armLength} },
				{ glm::vec2{0.0f, armOverlap}, glm::vec2{-armHWidth, -armLength}, glm::vec2{armHWidth, -armLength} },
				{ glm::vec2{-armOverlap, 0.0f}, glm::vec2{armLength, armHWidth}, glm::vec2{armLength, -armHWidth} },
				{ glm::vec2{armOverlap, 0.0f}, glm::vec2{-armLength, -armHWidth}, glm::vec2{-armLength, armHWidth} } },
				Tools::BodyParams().bodyType(b2_kinematicBody))).renderLayer = RenderLayer::NearMidground;
			staticWalls.last().texture = TCM::Texture(woodTexture);

			/*for (int i = 0; i < 4; ++i)
			{
				const float startAngle = glm::half_pi<float>() * i;

				auto& grapple = EmplaceDynamicComponent(Globals::Components().grapples(), { Tools::CreateCircleBody(1.0f, Tools::BodyParams().position(glm::vec2(glm::cos(startAngle), glm::sin(startAngle)) * grappleR).bodyType(b2_kinematicBody)),
					TCM::Texture(orbTexture) });
				grapple.influenceRadius = 15.0f;
				grapple.step = [&, grappleR, startAngle, angle = 0.0f]() mutable {
					const b2Vec2 pos = grapple.body->GetTransform().p;
					const b2Vec2 newPos(b2Vec2(glm::cos(startAngle + angle), glm::sin(startAngle + angle)) * grappleR);
					grapple.body->SetLinearVelocity((newPos - pos) / physics.frameDuration);
					angle += physics.frameDuration * rotationSpeed;
				};
			}*/

			constexpr float ringR = armLength + 15.0f;
			constexpr int numOfRingSegments = 50;
			constexpr float ringStep = glm::two_pi<float>() / numOfRingSegments;

			std::vector<glm::vec2> ringSegments;
			ringSegments.reserve(numOfRingSegments);
			for (int i = 0; i < numOfRingSegments; ++i)
			{
				ringSegments.emplace_back(glm::vec2(glm::cos(i * ringStep), glm::sin(i * ringStep)) * ringR);
			}
			ringSegments.push_back(ringSegments.front());

			auto& ring = Globals::Components().polylines().emplace(ringSegments, Tools::BodyParams().sensor(true));
			ring.segmentVerticesGenerator = [](const auto& v1, const auto& v2) {
				return Tools::CreateVerticesOfLightning(v1, v2, 10, 0.4f);
			};
			ring.keyVerticesTransformer = [rD = 0.4f](const auto& v) {
				return v + glm::vec3(Tools::Random(-rD, rD), Tools::Random(-rD, rD), 0.0f);
			};
			ring.loop = true;
			ring.colorF = []() {
				return glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) * 0.4f;
			};
		}

		void initHandlers()
		{
			playersHandler.initPlayers(rocketPlaneTexture, flameAnimatedTextureForPlayers, false,
				[this](unsigned player, auto) {
					return initPos(player);
				}, thrustSoundBuffer, grappleSoundBuffer);

			missilesHandler.setPlayersHandler(playersHandler);
			missilesHandler.setExplosionTexture(explosionTexture);
			missilesHandler.setMissileTexture(missileTexture);
			missilesHandler.setFlameAnimatedTexture(flameAnimatedTexture);
			missilesHandler.setExplosionParams(Tools::ExplosionParams().particlesDensity(0.2f).particlesRadius(2.0f).initExplosionVelocity(100.0f));
			missilesHandler.setExplosionF([this](auto pos) {
				Tools::PlaySingleSound(missileExplosionSoundBuffer, [pos](){ return pos; });
			});
		}

		void collisionHandlers()
		{
			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::plane, Globals::CollisionBits::polyline,
				[this](const auto& plane, auto) {
					Globals::Components().deferredActions().emplace([&](auto) {
						auto& planeComponent = *std::get<TCM::Plane>(Tools::AccessUserData(*plane.GetBody()).bodyComponentVariant).component;
						Tools::CreateExplosion(Tools::ExplosionParams().center(planeComponent.getCenter()).sourceVelocity(planeComponent.getVelocity()).
							initExplosionVelocityRandomMinFactor(0.2f).explosionTexture(explosionTexture));
						Tools::PlaySingleSound(playerExplosionSoundBuffer, [&]() { return planeComponent.getCenter(); });
						planeComponent.enable(false);
						return false;
					});
				});

			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::plane, Globals::CollisionBits::plane | Globals::CollisionBits::wall,
				[this](const auto& plane, const auto& obstacle) {
					Tools::PlaySingleSound(collisionSoundBuffer,
						[pos = *Tools::GetCollisionPoint(*plane.GetBody(), *obstacle.GetBody())]() {
							return pos;
						},
						[&](auto& sound) {
							sound.volume(Tools::GetRelativeVelocity(*plane.GetBody(), *obstacle.GetBody()) / 20.0f);
						});
				});
		}

		void step()
		{
			playersHandler.autodetectionStep([this](unsigned player) {
				const auto& windmill = Globals::Components().staticWalls()[windmillWall];
				return glm::rotate(glm::mat4(1.0f), windmill.getAngle(), { 0.0f, 0.0f, 1.0f }) * glm::vec4(initPos(player), 0.0f, 1.0f);
				});
			playersHandler.controlStep([this](unsigned playerHandlerId, bool fire) {
				missilesHandler.launchingMissile(playerHandlerId, fire, missileLaunchingSoundBuffer);
				});

			{
				const float rotationSpeed = 0.1f;
				const float innerForce = innerForceScale * 700.0f;

				const auto& physics = Globals::Components().physics();
				auto& windmill = Globals::Components().staticWalls()[windmillWall];
				auto& planes = Globals::Components().planes();

				windmill.body->SetTransform(windmill.body->GetPosition(), windmill.body->GetAngle() + physics.frameDuration * rotationSpeed);

				for (const auto& planeHandler : playersHandler.getPlayersHandlers())
				{
					auto& plane = planes[planeHandler.playerId];
					plane.body->ApplyForceToCenter(ToVec2<b2Vec2>(glm::normalize(plane.getCenter()) *
						(innerForce / glm::pow(glm::length(plane.getCenter()) - 8.0f * innerForceScale, 2.0f))), true);
				}

				innerForceScale += physics.frameDuration * 0.05f;

				Globals::Components().sounds()[innerForceSound].volume(innerForceScale / 3.0f);
			}
		}

	private:
		glm::vec2 initPos(unsigned player)
		{
			const float axesDistance = 20.0f;
			return glm::vec2(player == 0 || player == 3 ? axesDistance : -axesDistance, player == 0 || player == 1 ? axesDistance : -axesDistance);
		}

		ComponentId recursiveFaceRS = 0;

		ComponentId rocketPlaneTexture = 0;
		ComponentId woodTexture = 0;
		ComponentId orbTexture = 0;
		ComponentId explosionTexture = 0;
		ComponentId missileTexture = 0;
		ComponentId flameAnimationTexture = 0;
		ComponentId recursiveFaceAnimationTexture = 0;

		ComponentId playerExplosionSoundBuffer = 0;
		ComponentId missileExplosionSoundBuffer = 0;
		ComponentId missileLaunchingSoundBuffer = 0;
		ComponentId collisionSoundBuffer = 0;
		ComponentId thrustSoundBuffer = 0;
		ComponentId grappleSoundBuffer = 0;
		ComponentId innerForceSoundBuffer = 0;

		ComponentId innerForceSound = 0;

		std::array<unsigned, 4> flameAnimatedTextureForPlayers{ 0 };
		ComponentId flameAnimatedTexture = 0;
		ComponentId recursiveFaceAnimatedTexture = 0;

		ComponentId windmillWall = 0;

		Tools::PlayersHandler playersHandler;
		Tools::MissilesHandler missilesHandler;

		float innerForceScale = 0.0f;
	};

	Windmill::Windmill():
		impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->loadAudio();
		impl->setAnimations();
		impl->createBackground();
		impl->createForeground();
		impl->setCamera();
		impl->createWindmill();
		impl->initHandlers();
		impl->collisionHandlers();
	}

	Windmill::~Windmill() = default;

	void Windmill::step()
	{
		impl->step();
	}
}
