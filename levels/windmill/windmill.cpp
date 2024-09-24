#include "windmill.hpp"

#include <components/graphicsSettings.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/camera2D.hpp>
#include <components/plane.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/polyline.hpp>
#include <components/decoration.hpp>
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
#include <tools/shapes2D.hpp>
#include <tools/b2Helpers.hpp>
#include <tools/playersHandler.hpp>
#include <tools/missilesHandler.hpp>

#include <numeric>

namespace
{
	constexpr float armLength = 30.0f;
	constexpr float armOverlap = 5.0f;
	constexpr float armHWidth = 10.0f;
	constexpr float outerRingInitR = armLength + 15.0f;
}

namespace Levels
{
	class Windmill::Impl
	{
	public:
		Impl():
			physics(Globals::Components().physics())
		{
		}

		void setGraphicsSettings()
		{
			Globals::Components().graphicsSettings().defaultColorF = glm::vec4{ 0.7f, 0.7f, 0.7f, 1.0f };

			recursiveFaceRSF = [
				this,
				visibilityReduction = UniformsUtils::Uniform1b(),
				visibilityCenter = UniformsUtils::Uniform2f(),
				fullVisibilityDistance = UniformsUtils::Uniform1f(),
				invisibilityDistance = UniformsUtils::Uniform1f()
			](ShadersUtils::ProgramId program) mutable {
				if (!visibilityReduction.isValid())
				{
					visibilityReduction = UniformsUtils::Uniform1b(program, "visibilityReduction");
					visibilityCenter = UniformsUtils::Uniform2f(program, "visibilityCenter");
					fullVisibilityDistance = UniformsUtils::Uniform1f(program, "fullVisibilityDistance");
					invisibilityDistance = UniformsUtils::Uniform1f(program, "invisibilityDistance");
				}

				visibilityReduction(true);
				visibilityCenter({ 0.0f, 0.0f });
				fullVisibilityDistance(0.0f);
				invisibilityDistance(12.0f * innerForceScale);

				return [=]() mutable {
					visibilityReduction(false);
				};
			};
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().staticTextures();

			planeTextures[0] = textures.size();
			textures.emplace("textures/plane 1.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.6f, 1.8f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			planeTextures[1] = textures.size();
			textures.emplace("textures/alien ship 1.png");
			textures.last().translate = glm::vec2(-0.2f, 0.0f);
			textures.last().scale = glm::vec2(1.9f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			planeTextures[2] = textures.size();
			textures.emplace("textures/plane 2.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.8f, 1.8f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			planeTextures[3] = textures.size();
			textures.emplace("textures/alien ship 2.png");
			textures.last().translate = glm::vec2(0.0f, 0.0f);
			textures.last().scale = glm::vec2(1.45f, 1.4f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			spaceRockTexture = textures.size();
			textures.emplace("textures/space rock.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(20.0f);

			woodTexture = textures.size();
			textures.emplace("textures/wood.jpg", GL_MIRRORED_REPEAT);
			textures.last().scale = glm::vec2(20.0f);

			orbTexture = textures.size();
			textures.emplace("textures/orb.png");
			textures.last().scale = glm::vec2(2.0f);

			explosionTexture = textures.size();
			textures.emplace("textures/explosion.png");

			missileTexture = textures.size();
			textures.emplace("textures/missile 2.png");
			textures.last().minFilter = GL_LINEAR;
			textures.last().scale = glm::vec2(0.4f, 0.45f);
			textures.last().preserveAspectRatio = true;

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
			emissionSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Pad - Adventure_C.wav", 0.8f).getComponentId();
		}

		void setAnimations()
		{
			for (auto& flameAnimatedTextureForPlayer : flameAnimatedTextureForPlayers)
			{
				flameAnimatedTextureForPlayer = Globals::Components().staticAnimatedTextures().size();
				Globals::Components().staticAnimatedTextures().add({ CM::StaticTexture(flameAnimationTexture), { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
					AnimationData::Direction::Backward, AnimationData::Mode::Repeat, AnimationData::TextureLayout::Horizontal });
				Globals::Components().staticAnimatedTextures().last().start(true);
			}

			flameAnimatedTexture = Globals::Components().staticAnimatedTextures().size();
			Globals::Components().staticAnimatedTextures().add(Globals::Components().staticAnimatedTextures().last());

			recursiveFaceAnimatedTexture = Globals::Components().staticAnimatedTextures().size();
			Globals::Components().staticAnimatedTextures().add({ CM::StaticTexture(recursiveFaceAnimationTexture), { 263, 525 }, { 5, 10 }, { 0, 0 }, 210, 473, { 52, 52 }, 0.02f, 50, 0,
				AnimationData::Direction::Forward, AnimationData::Mode::Repeat, AnimationData::TextureLayout::Horizontal });
			Globals::Components().staticAnimatedTextures().last().start(true);
		}

		void createBackground() const
		{
			Tools::CreateJuliaBackground(Tools::JuliaParams{}.juliaCOffsetF([this]() {
				const auto averageCenter = std::accumulate(playersHandler.getPlayersHandlers().begin(), playersHandler.getPlayersHandlers().end(),
					glm::vec2(0.0f), [](const auto& acc, const auto& currentHandler) {
						return acc + Globals::Components().planes()[currentHandler.playerId].getOrigin2D();
					}) / (float)playersHandler.getPlayersHandlers().size();
					return averageCenter * 0.0001f; }));
		}

		void createForeground() const
		{
			auto& staticDecorations = Globals::Components().staticDecorations();

			staticDecorations.emplace(Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 15.0f, 15.0f }),
				CM::StaticAnimatedTexture(recursiveFaceAnimatedTexture), Shapes2D::CreateTexCoordOfRectangle(), std::move(recursiveFaceRSF), RenderLayer::NearForeground);
			staticDecorations.last().modelMatrixF = [&, angle = 0.0f]() mutable {
				return glm::rotate(glm::scale(glm::mat4(1.0f), glm::vec3(innerForceScale)), angle += 2.0f * physics.frameDuration, { 0.0f, 0.0f, 1.0f });
			};
			staticDecorations.last().colorF = []() {
				return glm::vec4(0.4f);
			};
		}

		void createWindmill()
		{
			auto& staticWalls = Globals::Components().staticWalls();

			windmillWall = staticWalls.size();
			staticWalls.emplace(Tools::CreateTrianglesBody({ 
				{ glm::vec2{0.0f, -armOverlap}, glm::vec2{armHWidth, armLength}, glm::vec2{-armHWidth, armLength} },
				{ glm::vec2{0.0f, armOverlap}, glm::vec2{-armHWidth, -armLength}, glm::vec2{armHWidth, -armLength} },
				{ glm::vec2{-armOverlap, 0.0f}, glm::vec2{armLength, armHWidth}, glm::vec2{armLength, -armHWidth} },
				{ glm::vec2{armOverlap, 0.0f}, glm::vec2{-armLength, -armHWidth}, glm::vec2{-armLength, armHWidth} } },
				Tools::BodyParams().bodyType(b2_kinematicBody)));
			staticWalls.last().texture = CM::StaticTexture(woodTexture);
		}

		void createOuterRing()
		{
			auto& outerRing = Globals::Components().staticPolylines().emplace();

			outerRing.stepF = [&]() {
				constexpr int numOfRingSegments = 50;
				constexpr float ringStep = glm::two_pi<float>() / numOfRingSegments;
				float deltaR = glm::sin((physics.simulationDuration - startTime) * 0.04f) * 50.0f;
				outerRingR = outerRingInitR + deltaR;

				std::vector<glm::vec2> ringSegments;
				ringSegments.reserve(numOfRingSegments);
				for (int i = 0; i < numOfRingSegments; ++i)
				{
					ringSegments.emplace_back(glm::vec2(glm::cos(i * ringStep), glm::sin(i * ringStep)) * outerRingR);
				}
				ringSegments.push_back(ringSegments.front());

				outerRing.replaceFixtures(ringSegments, Tools::BodyParams().sensor(true));
			};

			outerRing.segmentVerticesGenerator = [](const auto& v1, const auto& v2) { return Shapes2D::CreateVerticesOfLightning(v1, v2, 10, 0.2f); };
			outerRing.keyVerticesTransformer = [](std::vector<glm::vec3>& vertices) { Tools::VerticesDefaultRandomTranslate(vertices, true, 0.04f); };
			outerRing.colorF = [this]() {
				return (playersHandler.getActivePlayersHandlers().size() == 1
					? glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
					: glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)) * 0.4f;
			};
		}

		void createGrapples()
		{
			constexpr float grappleRDist = armLength + 5.0f;

			/*for (int i = 0; i < 4; ++i)
			{
				const float startAngle = glm::half_pi<float>() * i;

				auto& grapple = Globals::Components().grapples().emplace(Tools::CreateCircleBody(1.0f, Tools::BodyParams().position(glm::vec2(glm::cos(startAngle), glm::sin(startAngle)) * grappleRDist).bodyType(b2_kinematicBody)),
					TCM::StaticTexture(orbTexture));
				grapple.influenceRadius = 15.0f;
				grapple.stepF = [&, grappleRDist, startAngle, angle = 0.0f, rotationSpeed = 0.2f]() mutable {
					const b2Vec2 pos = grapple.body->GetTransform().p;
					const b2Vec2 newPos(b2Vec2(glm::cos(startAngle + angle), glm::sin(startAngle + angle)) * grappleRDist);
					grapple.body->SetLinearVelocity((newPos - pos) / physics.frameDuration);
					angle += physics.frameDuration * rotationSpeed;
				};
			}*/

			/*for (int i = 0; i < 4; ++i)
			{
				const float startAngle = glm::half_pi<float>() * i;

				auto& grapple = Globals::Components().grapples().emplace(Tools::CreateCircleBody(1.0f, Tools::BodyParams().position(glm::vec2(glm::cos(startAngle), glm::sin(startAngle)) * grappleRDist)
					.bodyType(b2_dynamicBody).density(10.0f)), TCM::StaticTexture(orbTexture));
				grapple.influenceRadius = 15.0f;
				grapplesDebris.push_back(grapple.getComponentId());
			}*/
		}

		void createDebris()
		{
			constexpr int numOfDebris = 8;
			auto& dynamicWalls = Globals::Components().dynamicWalls();

			for (const auto debrisId : dynamicWallsDebris)
			{
				dynamicWalls[debrisId].setEnabled(false);
				dynamicWalls[debrisId].state = ComponentState::Outdated;
			}

			dynamicWallsDebris.clear();

			for (int i = 0; i < numOfDebris; ++i)
			{
				const float angle = i / (float)numOfDebris * glm::two_pi<float>();
				glm::vec2 center(glm::cos(angle), glm::sin(angle));
				center *= (outerRingInitR + armLength) / 2.0f;
				dynamicWalls.emplace(Tools::CreateRandomPolygonBody(12, 5.0f,
					Tools::BodyParams().bodyType(b2_dynamicBody).position(center).density(2.0f).linearDamping(0.1f).angularDamping(0.1f)));
				dynamicWalls.last().texture = CM::StaticTexture(spaceRockTexture);

				dynamicWallsDebris.push_back(dynamicWalls.last().getComponentId());
			}
		}

		void initHandlers()
		{
			playersHandler.setCamera(Tools::PlayersHandler::CameraParams().projectionHSizeMin([]() { return 30.0f; }).scalingFactor(0.7f).additionalActors([]() { return glm::vec2(0.0f); }));

			missilesHandler.setPlayersHandler(playersHandler);
			missilesHandler.setExplosionTexture(explosionTexture);
			missilesHandler.setMissileTexture(missileTexture);
			missilesHandler.setFlameAnimatedTexture(flameAnimatedTexture);
			missilesHandler.setExplosionParams(Tools::ExplosionParams().particlesDensity(0.2f).particlesRadius(2.0f).initExplosionVelocity(100.0f));
			missilesHandler.setExplosionF([this](auto pos) {
				Tools::CreateAndPlaySound(missileExplosionSoundBuffer, [pos](){ return pos; });
			});
		}

		void collisionHandlers()
		{
			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::plane, Globals::CollisionBits::polyline,
				[this](const auto& plane, auto) {
					if (playersHandler.getActivePlayersHandlers().size() == 1)
						return;

					Globals::Components().deferredActions().emplace([&](auto) {
						auto& planeComponent = *std::get<CM::Plane>(Tools::AccessUserData(*plane.GetBody()).bodyComponentVariant).component;
						Tools::CreateExplosion(Tools::ExplosionParams().center(planeComponent.getOrigin2D()).sourceVelocity(planeComponent.getVelocity()).
							initExplosionVelocityRandomMinFactor(0.2f).explosionTexture(explosionTexture));
						Tools::CreateAndPlaySound(playerExplosionSoundBuffer, [pos = planeComponent.getOrigin2D()]() { return pos; });
						planeComponent.setEnabled(false);
						return false;
						});
				});

			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::plane, Globals::CollisionBits::plane | Globals::CollisionBits::wall,
				Tools::SkipDuplicatedBodiesCollisions([this](const auto& plane, const auto& obstacle) {
					Tools::CreateAndPlaySound(collisionSoundBuffer,
						[pos = *Tools::GetCollisionPoint(*plane.GetBody(), *obstacle.GetBody())]() {
							return pos;
						},
						[&](auto& sound) {
							sound.setVolume(std::sqrt(Tools::GetRelativeVelocity(*plane.GetBody(), *obstacle.GetBody()) / 20.0f));
						});
				}));

			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::wall, Globals::CollisionBits::wall,
				[this, soundsLimitter = Tools::SoundsLimitter::create(8)](const auto& wall1, const auto& wall2) mutable {
					soundsLimitter->newSound(Tools::CreateAndPlaySound(collisionSoundBuffer,
						[pos = *Tools::GetCollisionPoint(*wall1.GetBody(), *wall2.GetBody())]() {
							return pos;
						},
						[&](auto& sound) {
							sound.setVolume(std::sqrt(Tools::GetRelativeVelocity(*wall1.GetBody(), *wall2.GetBody()) / 20.0f));
							sound.setPitch(0.5f);
						}));
				});
		}

		void step()
		{
			playersHandler.gamepadsAutodetectionStep([this](unsigned player) {
				const auto& windmill = Globals::Components().staticWalls()[windmillWall];
				glm::vec3 initLoc = this->initLoc(player);
				return glm::rotate(glm::mat4(1.0f), windmill.getAngle(), { 0.0f, 0.0f, 1.0f }) * glm::vec4(glm::vec2(initLoc),
					windmill.getAngle() + initLoc.z, 1.0f);
				});

			playersHandler.controlStep([this](unsigned playerHandlerId, bool fire) {
				missilesHandler.launchingMissile(playerHandlerId, fire, missileLaunchingSoundBuffer);
				if (playersHandler.getActivePlayersHandlers().size() == 1 && Globals::Components().planes()[playersHandler.getActivePlayersHandlers().front()->playerId].controls.startPressed)
					reset();
				});

			windmillRotation();
			forceFieldsStep();
			emissions();
		}

		void reset()
		{
			innerForceScale = 0.0f;
			nextEmissionTime = emissionInterval;
			startTime = physics.simulationDuration;
			missilesHandler.removeActiveMissiles();
			Globals::MarkDynamicComponentsAsDirty();

			innerForceSound = Tools::CreateAndPlaySound(innerForceSoundBuffer, []() { return glm::vec2(0.0f); },
				[](auto& sound) {
					sound.setLoop(true);
					sound.setVolume(0.0f);
				}).getComponentId();

			const auto& windmill = Globals::Components().staticWalls()[windmillWall];
			windmill.body->SetTransform({ 0.0f, 0.0f }, 0.0f);

			playersHandler.initPlayers(planeTextures, flameAnimatedTextureForPlayers, false,
				[this](unsigned playerId, auto) {
					return initLoc(playerId);
				}, false, thrustSoundBuffer, grappleSoundBuffer);

			missilesHandler.initCollisions();

			createGrapples();
			createDebris();
			collisionHandlers();
		}

	private:
		glm::vec3 initLoc(unsigned playerId)
		{
			const float axesDistance = 20.0f;
			return glm::vec3(playerId == 0 || playerId == 3 ? axesDistance : -axesDistance, playerId == 0 || playerId == 1 ? axesDistance : -axesDistance,
				playerId * glm::half_pi<float>() + glm::quarter_pi<float>());
		}

		void windmillRotation() const
		{
			const float rotationSpeed = 0.1f;
			auto& windmill = Globals::Components().staticWalls()[windmillWall];

			windmill.body->SetTransform(windmill.body->GetPosition(), windmill.body->GetAngle() + physics.frameDuration * rotationSpeed);
		}

		void forceFieldsStep()
		{
			const float orbitR = (armLength + outerRingR) / 2.0f;

			for (const auto& planeHandler : playersHandler.getPlayersHandlers())
			{
				const float innerForceForPlanes = innerForceScale * 700.0f;
				auto& plane = Globals::Components().planes()[planeHandler.playerId];
				plane.body->ApplyForceToCenter(ToVec2<b2Vec2>(glm::normalize(plane.getOrigin2D()) *
					(innerForceForPlanes / glm::pow(glm::length(plane.getOrigin2D()) - 8.0f * innerForceScale, 2.0f))), true);
			}

			auto applyForceForDebris = [&](auto& debris, float forceFactor) {
				debris.body->ApplyForceToCenter(ToVec2<b2Vec2>(glm::normalize(debris.getOrigin2D()) *
					(orbitR - glm::length(debris.getOrigin2D())) * forceFactor), true);
			};

			for (auto id : grapplesDebris)
				applyForceForDebris(Globals::Components().grapples()[id], 100.0f);
			for (auto id : dynamicWallsDebris)
				applyForceForDebris(Globals::Components().dynamicWalls()[id], 10.0f);

			innerForceScale += physics.frameDuration * 0.05f;

			Globals::Components().sounds()[innerForceSound].setVolume(innerForceScale / 3.0f);
		}

		void emissions()
		{
			auto& polylines = Globals::Components().dynamicPolylines();

			if (physics.simulationDuration >= nextEmissionTime)
			{
				auto& emission = polylines.emplace();
				auto stop = std::make_shared<bool>(false);
				emission.stepF = [&, emissionR = 0.0f, stop]() mutable {
					constexpr int numOfRingSegments = 50;
					constexpr float ringStep = glm::two_pi<float>() / numOfRingSegments;

					emissionR += physics.frameDuration * 5.0f;

					if (emissionR > innerForceScale * 10.0f)
					{
						*stop = true;
						emission.state = ComponentState::Outdated;
						return;
					}

					std::vector<glm::vec2> ringSegments;
					ringSegments.reserve(numOfRingSegments);
					for (int i = 0; i < numOfRingSegments; ++i)
					{
						ringSegments.emplace_back(glm::vec2(glm::cos(i * ringStep), glm::sin(i * ringStep)) * emissionR);
					}
					ringSegments.push_back(ringSegments.front());

					emission.replaceFixtures(ringSegments, Tools::BodyParams().sensor(true));
				};

				emission.segmentVerticesGenerator = [](const auto& v1, const auto& v2) { return Shapes2D::CreateVerticesOfLightning(v1, v2, 10, 0.2f); };
				emission.keyVerticesTransformer = [](std::vector<glm::vec3>& vertices) { Tools::VerticesDefaultRandomTranslate(vertices, true, 0.04f); };
				emission.colorF = [this]() {
					return (playersHandler.getActivePlayersHandlers().size() == 1
						? glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
						: glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)) * 0.4f;
				};

				Tools::CreateAndPlaySound(emissionSoundBuffer, []() { return glm::vec2(0.0f); }, nullptr,
					[this, stop, volume = 1.0f](auto& sound) mutable {
						if (*stop)
						{
							volume -= physics.frameDuration * 2.0f;
							sound.setVolume(volume);
							if (volume <= 0.0f)
								sound.stop();
						}
					});

				nextEmissionTime += emissionInterval;
			}
		}

		const Components::Physics& physics;

		const float emissionInterval = 10.0f;

		RenderableDef::RenderingSetupF recursiveFaceRSF;

		std::array<ComponentId, 4> planeTextures{ 0 };
		ComponentId spaceRockTexture = 0;
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
		ComponentId emissionSoundBuffer = 0;

		ComponentId innerForceSound = 0;

		std::array<ComponentId, 4> flameAnimatedTextureForPlayers{ 0 };
		ComponentId flameAnimatedTexture = 0;
		ComponentId recursiveFaceAnimatedTexture = 0;

		ComponentId windmillWall = 0;

		std::vector<ComponentId> dynamicWallsDebris;
		std::vector<ComponentId> grapplesDebris;

		Tools::PlayersHandler playersHandler;
		Tools::MissilesHandler missilesHandler;

		float startTime = 0.0f;
		float innerForceScale = 0.0f;
		float outerRingR = 0.0f;
		float nextEmissionTime = emissionInterval;
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
		impl->createWindmill();
		impl->createOuterRing();
		impl->initHandlers();

		impl->reset();
	}

	Windmill::~Windmill() = default;

	void Windmill::step()
	{
		impl->step();
	}
}
