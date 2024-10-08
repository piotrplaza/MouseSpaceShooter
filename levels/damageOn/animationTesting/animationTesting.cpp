#include "animationTesting.hpp"

#include <components/graphicsSettings.hpp>
#include <components/camera2D.hpp>
#include <components/keyboard.hpp>
#include <components/gamepad.hpp>
#include <components/texture.hpp>
#include <components/animatedTexture.hpp>
#include <components/actor.hpp>
#include <components/wall.hpp>
#include <components/decoration.hpp>
#include <components/music.hpp>
#include <components/soundBuffer.hpp>
#include <components/sound.hpp>
#include <components/audioListener.hpp>
#include <components/physics.hpp>
#include <components/pauseHandler.hpp>
#include <globals/components.hpp>

#include <tools/Shapes2D.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>
#include <tools/glmHelpers.hpp>

#include <ogl/uniformsUtils.hpp>

#include <glm/gtc/random.hpp>

#include <array>
#include <unordered_map>
#include <fstream>
#include <iostream>

namespace Levels::DamageOn
{
	struct GameParams
	{
		int numOfPlayers;
		float globalVolume;
		float musicVolume;
		float mapHSize;
	};

	struct PlayerParams
	{
		glm::vec2 startPosition;
		float radius;
		float maxVelocity;
		float density;
		float linearDamping;
		float dash;

		struct Presentation
		{
			glm::vec2 radiusProportions;
			glm::vec2 translation;
			float rotation;
			glm::vec2 scale;
			float velocityRotationFactor;
			glm::vec2 weaponOffset;
		} presentation;

		struct Animation
		{
			glm::ivec2 textureSize;
			glm::ivec2 framesGrid;
			glm::ivec2 leftTopFrameLeftTopCorner;
			int rightTopFrameLeftEdge;
			int leftBottomFrameTopEdge;
			glm::ivec2 frameSize;
			float frameDuration;
			int numOfFrames;
			int startFrame;
			AnimationData::Direction direction;
			AnimationData::Mode mode;
			AnimationData::TextureLayout textureLayout;
			glm::vec2 frameTranslation;
			float frameRotation;
			glm::vec2 frameScale;
			int neutralFrame;
		} animation;
	};

	struct SparkingParams
	{
		float distance;
		float damageFactor;
		float overheatingRate;
		float coolingRate;
	};

	struct EnemyGhostParams
	{
		int initCount;
		glm::vec2 initSpawnPosition;
		float initHP;
		float initRadius;
		float density;
		float baseVelocity;
		float boostDistance;
		float boostFactor;
		float slowFactor;
		float radiusReductionFactor;
		float minimalRadius;
		int killSpawns;
	};

	namespace
	{
		constexpr const char* paramsPath = "levels/damageOn/animationTesting/params.txt";

		constexpr int debrisCount = 10;
		constexpr float debrisDensity = 100.0f;
		constexpr float gamepadDeadZone = 0.2f;
		constexpr float gamepadTriggerDeadZone = 0.1f;

		RenderableDef::RenderingSetupF createRecursiveFaceRS(glm::vec2 fadingRange)
		{
			return { [=,
				colorUniform = UniformsUtils::Uniform4f(),
				visibilityReduction = UniformsUtils::Uniform1b(),
				fullVisibilityDistance = UniformsUtils::Uniform1f(),
				invisibilityDistance = UniformsUtils::Uniform1f()
			] (ShadersUtils::ProgramId program) mutable {
				if (!colorUniform.isValid())
				{
					colorUniform = UniformsUtils::Uniform4f(program, "color");
					visibilityReduction = UniformsUtils::Uniform1b(program, "visibilityReduction");
					fullVisibilityDistance = UniformsUtils::Uniform1f(program, "fullVisibilityDistance");
					invisibilityDistance = UniformsUtils::Uniform1f(program, "invisibilityDistance");
				}

				visibilityReduction(true);
				fullVisibilityDistance(fadingRange.x);
				invisibilityDistance(fadingRange.y);

				return [=]() mutable {
					visibilityReduction(false);
				};
			} };
		}
	}

	class AnimationTesting::Impl
	{
	public:
		void setup()
		{
			loadParams();

			auto& graphicsSettings = Globals::Components().graphicsSettings();
			graphicsSettings.lineWidth = 10.0f;

			auto& textures = Globals::Components().staticTextures();
			auto& animatedTextures = Globals::Components().dynamicAnimatedTextures();

			backgroundTextureId = textures.emplace("textures/damageOn/head.jpg", GL_CLAMP_TO_BORDER).getComponentId();
			textures.last().magFilter = GL_NEAREST;
			textures.last().scale = glm::vec2(1.0f);
			textures.last().preserveAspectRatio = true;

			coffinTextureId = textures.emplace("textures/damageOn/coffin.png", GL_MIRRORED_REPEAT).getComponentId();
			textures.last().magFilter = GL_NEAREST;
			//textures.last().scale = glm::vec2(30.0f);

			fogTextureId = textures.size();
			textures.emplace("textures/damageOn/fog.png", GL_REPEAT);
			textures.last().scale = glm::vec2(0.15f);

			playerAnimationTextureId = textures.emplace("textures/damageOn/player.png").getComponentId();
			textures.last().magFilter = GL_NEAREST;

			enemyAnimationTextureId = textures.emplace(TextureFile("textures/damageOn/enemy.jpg", 4, true, TextureFile::AdditionalConversion::DarkToTransparent, [](float* data, glm::ivec2 size, int numOfChannels) {
				for (int i = 0; i < size.x * size.y; ++i)
				{
					glm::vec4& pixel = reinterpret_cast<glm::vec4*>(data)[i];
					if (pixel.r + pixel.g + pixel.b < 0.2f)
						pixel = {};
				}
			})).getComponentId();
			textures.last().minFilter = GL_NEAREST;
			
			for (auto& enemyAnimatedTextureId : enemyAnimatedTextureIds)
			{
				enemyAnimatedTextureId = animatedTextures.add({ CM::StaticTexture(enemyAnimationTextureId), { 263, 525 }, { 5, 10 }, { 0, 0 }, 210, 473, { 52, 52 }, 0.02f, 50, Tools::RandomInt(0, 49),
					AnimationData::Direction::Backward, AnimationData::Mode::Repeat, AnimationData::TextureLayout::Horizontal }).getComponentId();
				animatedTextures.last().start(true);
			}

			auto& soundsBuffers = Globals::Components().soundsBuffers();
			auto& sounds = Globals::Components().sounds();

			sparkingSoundId = sounds.emplace(soundsBuffers.emplace("audio/Ghosthack Synth - Choatic_C.wav").getComponentId()).getComponentId();
			sounds.last().setLoop(true).play().pause();
			overchargeSoundId = sounds.emplace(soundsBuffers.emplace("audio/Ghosthack Scrape - Horror_C.wav").getComponentId()).getComponentId();
			sounds.last().setLoop(true).play().pause();
			dashSoundId = sounds.emplace(soundsBuffers.emplace("audio/Ghosthack Whoosh - 5.wav").getComponentId()).getComponentId();
			sounds.last().setVolume(0.15f);

			killSoundBufferId = soundsBuffers.emplace("audio/Ghosthack Impact - Edge.wav").getComponentId();

			Tools::CreateFogForeground(2, 0.1f, fogTextureId, glm::vec4(1.0f), [x = 0.0f](int layer) mutable {
				(void)layer;
				const auto& physics = Globals::Components().physics();
				x += physics.frameDuration * 0.01f;
				const float y = std::sin(x * 30) * 0.01f;
				return glm::vec2(x, y);
			});

			auto& musics = Globals::Components().musics();
			musics.emplace("audio/Damage On.ogg").play();
			musicId = musics.last().getComponentId();

			Globals::Components().pauseHandler().handler = [&](bool prevPauseState) {
				auto& audioListener = Globals::Components().audioListener();
				audioListener.setEnabled(!audioListener.isEnabled());

				if (musics.last().isPlaying())
					musics.last().pause();
				else
					musics.last().play();

				return !prevPauseState;
			};

			reload(false);
		}

		void postSetup()
		{
			auto& camera = Globals::Components().camera2D();
			auto& walls = Globals::Components().staticWalls();
			auto& decorations = Globals::Components().staticDecorations();
			auto& textures = Globals::Components().staticTextures();
			auto& physics = Globals::Components().physics();

			const glm::vec2 levelHSize(textures[backgroundTextureId].loaded.getAspectRatio() * gameParams.mapHSize, gameParams.mapHSize);
			camera.targetProjectionHSizeF = camera.details.projectionHSize = camera.details.prevProjectionHSize = 10.0f;
			camera.targetPositionF = [&, levelHSize]() { return glm::clamp(playerIdsToData.begin()->second.actor.getOrigin2D(),
				-levelHSize + camera.details.completeProjectionHSize, levelHSize - camera.details.completeProjectionHSize); };
			camera.positionTransitionFactor = 5.0f;

			const float borderHThickness = 10.0f;
			walls.emplace(Tools::CreateBoxBody({ levelHSize.x + borderHThickness, borderHThickness }, Tools::BodyParams{}.position({ 0.0f, -levelHSize.y - borderHThickness }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			walls.emplace(Tools::CreateBoxBody({ levelHSize.x + borderHThickness, borderHThickness }, Tools::BodyParams{}.position({ 0.0f, levelHSize.y + borderHThickness }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			walls.emplace(Tools::CreateBoxBody({ borderHThickness, levelHSize.y + borderHThickness }, Tools::BodyParams{}.position({ -levelHSize.x - borderHThickness, 0.0f }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			walls.emplace(Tools::CreateBoxBody({ borderHThickness, levelHSize.y + borderHThickness }, Tools::BodyParams{}.position({ levelHSize.x + borderHThickness, 0.0f }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			const float mapScaleFactor = gameParams.mapHSize / 20.0f;
			const glm::vec2 headCenter = glm::vec2(1.8f, 2.6f) * mapScaleFactor;
			walls.emplace(Tools::CreateCircleBody(6.0f * mapScaleFactor, Tools::BodyParams{}.position(headCenter))).renderF = false;
			walls.emplace(Tools::CreateCircleBody(5.5f * mapScaleFactor, Tools::BodyParams{}.position(headCenter + glm::vec2(-2.5f) * mapScaleFactor))).renderF = false;
			walls.emplace(Tools::CreateCircleBody(3.0f * mapScaleFactor, Tools::BodyParams{}.position(headCenter + glm::vec2(-6.0f) * mapScaleFactor))).renderF = false;

			auto screenCordTexturesF = [sceneCoordTextures = UniformsUtils::Uniform1b()](ShadersUtils::ProgramId program) mutable {
				if (!sceneCoordTextures.isValid())
					sceneCoordTextures = UniformsUtils::Uniform1b(program, "sceneCoordTextures");
				sceneCoordTextures(true);
				return [=]() mutable { sceneCoordTextures(false); };
			};

			//for (auto sign : { -1, 1 })
			//	walls.emplace(Tools::CreateBoxBody(glm::vec2(2.0f), Tools::BodyParams{}.position({ sign * levelHSize.x / 2.5f, 0.0f })), CM::StaticTexture(greenMarbleTextureId)).renderingSetupF = screenCordTexturesF;

			for (int i = 0; i < debrisCount; ++i)
			{
				const float debrisWidth = glm::linearRand(0.3f, 1.0f);
				const float debrisHeight = debrisWidth * glm::linearRand(1.6f, 2.0f);
				walls.emplace(Tools::CreateBoxBody({ debrisWidth, debrisHeight }, Tools::BodyParams{}.position(glm::linearRand(-levelHSize, levelHSize))
					.bodyType(b2_dynamicBody).linearDamping(10.0f).angularDamping(10.0f).density(debrisDensity)), CM::StaticTexture(coffinTextureId));
				walls.last().texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
			}

			for (int i = 0; i < enemyGhostParams.initCount; ++i)
				enemySpawn(enemyGhostParams.initSpawnPosition + glm::circularRand(0.1f), enemyGhostParams.initRadius);

			decorations.emplace(Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, levelHSize), CM::StaticTexture(backgroundTextureId), Tools::Shapes2D::CreateTexCoordOfRectangle()).renderLayer = RenderLayer::FarBackground;
		}

		void step()
		{
			auto& keyboard = Globals::Components().keyboard();
			auto& gamepad = Globals::Components().gamepads()[0];
			auto& physics = Globals::Components().physics();

			if (keyboard.pressed['P'] || gamepad.pressed.start)
				reload();

			for (auto& [playerId, playerData] : playerIdsToData)
			{
				for (auto& [enemyId, enemyData] : enemyIdsToData)
				{
					const auto direction = playerData.actor.getOrigin2D() - enemyData.actor.getOrigin2D();
					const auto distance = glm::length(direction);
					if (distance > 0.0f)
						enemyData.actor.setVelocity(direction / distance * enemyGhostParams.baseVelocity);
				}

				const glm::vec2 gamepadDirection = [&]() {
					if (!gamepad.enabled)
						return glm::vec2(0.0f);
					const float directionLength = glm::length(gamepad.lStick);
					if (directionLength < gamepadDeadZone)
						return glm::vec2(0.0f);
					if (directionLength > 1.0f)
						return gamepad.lStick / directionLength;
					return gamepad.lStick;
					}();

				const glm::vec2 keyboardDirection = [&]() {
					const glm::vec2 direction(-(int)keyboard.pressing[/*VK_LEFT*/0x25] + (int)keyboard.pressing[/*VK_RIGHT*/0x27], -(int)keyboard.pressing[/*VK_DOWN*/0x28] + (int)keyboard.pressing[/*VK_UP*/0x26]);
					if (direction == glm::vec2(0.0f))
						return glm::vec2(0.0f);
					return direction / glm::length(direction);
					}();

				const glm::vec2 direction = [&]() {
					const glm::vec2 direction = gamepadDirection + keyboardDirection;
					if (glm::length(direction) > 1.0f)
						return direction / glm::length(direction);
					return direction;
					}();

				playerData.animatedTexture.setSpeedScaling(glm::length(playerData.actor.getVelocity()));
				if (playerData.animatedTexture.isForcingFrame())
				{
					playerData.animatedTexture.forceFrame(std::nullopt);
					playerData.animatedTexture.start(true);
				}

				if (direction.x < 0.0f)
					playerData.sideFactor = -1.0f;
				else if (direction.x > 0.0f)
					playerData.sideFactor = 1.0f;
				else if (direction.y == 0.0f)
					playerData.animatedTexture.forceFrame(playerParams.animation.neutralFrame);

				playerData.animatedTexture.setAdditionalTransformation(playerParams.animation.frameTranslation * glm::vec2(playerData.sideFactor, 1.0f),
					playerParams.animation.frameRotation * playerData.sideFactor, playerParams.animation.frameScale * glm::vec2(playerData.sideFactor, 1.0f));

				const glm::vec2 newVelocity = direction * playerParams.maxVelocity;
				if (glm::length(newVelocity) > glm::length(playerData.actor.getVelocity()))
					playerData.actor.setVelocity(newVelocity);

				if (keyboard.pressing[/*VK_CONTROL*/0x11] || gamepad.rTrigger > gamepadTriggerDeadZone || gamepad.lTrigger > gamepadTriggerDeadZone)
				{
					playerData.fire = true;
					playerData.autoFire = false;
				}
				else
					playerData.fire = false;

				if (keyboard.pressed[/*VK_SHIFT*/0x10] || gamepad.pressed.rShoulder || gamepad.pressed.lShoulder)
					playerData.autoFire = !playerData.autoFire;

				if ((keyboard.pressed[/*VK_SPACE*/0x20] || gamepad.pressed.a) && glm::length(direction) > 0.0f)
				{
					auto& dashSound = Globals::Components().sounds()[dashSoundId];
					dashSound.stop().setPlayingOffset(0.35f).setPosition(playerData.actor.getOrigin2D()).play();
					playerData.actor.body->ApplyLinearImpulseToCenter(ToVec2<b2Vec2>(direction * playerParams.dash * playerData.actor.body->GetMass()), true);
				}

				if (keyboard.pressed['T'] || gamepad.pressed.y)
					playerData.presentationTransparency = !playerData.presentationTransparency;
				if (keyboard.pressed['B'] || gamepad.pressed.x)
					playerData.bodyRendering = !playerData.bodyRendering;

				sparkingHandler(playerData, playerData.fire || playerData.autoFire);

				const float vLength = glm::length(playerData.actor.getVelocity());
				playerData.angle = -glm::min(glm::quarter_pi<float>(), (vLength * vLength * playerParams.presentation.velocityRotationFactor));
			}
		}

	private:
		void playerSpawn(int playerId, glm::vec2 position)
		{
			const auto playerPresentationSize = playerParams.presentation.radiusProportions * playerParams.radius;

			const auto& physics = Globals::Components().physics();
			
			auto& playerAnimatedTexture = Globals::Components().dynamicAnimatedTextures().emplace();
			playerAnimatedTexture.setAnimationData({ CM::StaticTexture(playerAnimationTextureId), playerParams.animation.textureSize, playerParams.animation.framesGrid, playerParams.animation.leftTopFrameLeftTopCorner,
				playerParams.animation.rightTopFrameLeftEdge, playerParams.animation.leftBottomFrameTopEdge, playerParams.animation.frameSize, playerParams.animation.frameDuration, playerParams.animation.numOfFrames,
				playerParams.animation.startFrame == -1 ? glm::linearRand(0, playerParams.animation.numOfFrames - 1) : playerParams.animation.startFrame,
				playerParams.animation.direction, playerParams.animation.mode, playerParams.animation.textureLayout });
			playerAnimatedTexture.start(true);

			auto& playerActor = Globals::Components().actors().emplace(Tools::CreateCircleBody(playerParams.radius, Tools::BodyParams{}.linearDamping(playerParams.linearDamping).fixedRotation(true).bodyType(b2_dynamicBody).density(playerParams.density).position(position)),
				CM::DummyTexture{});
			const auto& playerData = playerIdsToData.emplace(playerId, PlayerData{ playerActor, playerAnimatedTexture }).first->second;

			playerActor.renderF = [&]() { return playerData.bodyRendering; };
			playerActor.colorF = glm::vec4(0.4f);
			playerActor.posInSubsequence = 1;

			auto& playerPresentation = playerActor.subsequence.emplace_back();
			playerPresentation.renderingSetupF = [&](auto) { if (!playerData.presentationTransparency) glDisable(GL_BLEND); return []() mutable { glEnable(GL_BLEND); }; };
			playerPresentation.vertices = Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, playerPresentationSize);
			playerPresentation.texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
			playerPresentation.texture = CM::DynamicAnimatedTexture(playerData.animatedTexture.getComponentId());
			playerPresentation.modelMatrixF = [&]() {
				return playerActor.modelMatrixF() * glm::translate(glm::mat4(1.0f), glm::vec3(playerParams.presentation.translation, 0.0f) * glm::vec3(playerData.sideFactor, 1.0f, 1.0f))
					* glm::rotate(glm::mat4(1.0f), (playerParams.presentation.rotation + playerData.angle) * playerData.sideFactor, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::scale(glm::mat4(1.0f), glm::vec3(playerParams.presentation.scale, 1.0f));
			};
			playerPresentation.colorF = [&]() {
				return glm::mix(glm::vec4(1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), sparkingOverheating) * (sparkingOverheated ? (glm::sin(physics.simulationDuration * 20.0f) + 1.0f) / 2.0f : 1.0f);
			};
		}

		void enemySpawn(glm::vec2 position, float radius)
		{
			auto& actors = Globals::Components().actors();
			auto& physics = Globals::Components().physics();
			auto& enemyActor = actors.emplace(Tools::CreateCircleBody(radius, Tools::BodyParams{ defaultBodyParams }
				.bodyType(b2_dynamicBody)
				.density(enemyGhostParams.density)
				.position(position)), CM::DynamicAnimatedTexture(enemyAnimatedTextureIds[enemyIdsToData.size() % enemyAnimatedTextureIds.size()], {}, {}, glm::vec2(2.6f * radius)));
			const auto enemyId = enemyIdGenerator.acquire();
			auto& enemyData = enemyIdsToData.emplace(enemyId, EnemyData{ enemyActor }).first->second;
			auto damageColorFactor = std::make_shared<glm::vec4>(1.0f);

			enemyActor.renderingSetupF = createRecursiveFaceRS({ radius * 0.6f, radius });
			enemyActor.colorF = [baseColor = glm::vec4(glm::vec3(glm::linearRand(0.0f, 1.0f)), 1.0f) * 0.8f, damageColorFactor]() { return glm::mix(baseColor, *damageColorFactor, 0.5f); };
			enemyActor.stepF = [&,
				enemyId,
				radius,
				sparks = std::array<Components::Decoration*, 4>{ &Globals::Components().dynamicDecorations().emplace(), &Globals::Components().dynamicDecorations().emplace(),
					&Globals::Components().dynamicDecorations().emplace(), &Globals::Components().dynamicDecorations().emplace() },
				damageColorFactor,
				hp = enemyGhostParams.initHP]() mutable {
				bool kill = false;
				for (auto& [playerId, playerData] : playerIdsToData)
				{
					const auto direction = playerData.actor.getOrigin2D() - enemyActor.getOrigin2D();
					const auto distance = glm::length(direction);

					enemyBoost(enemyActor, direction, distance);
					if (sparkHandler(playerData, enemyData, *sparks[playerId], direction, distance, *damageColorFactor, playerData.fire || playerData.autoFire))
					{
						++playerData.activeSparks;
						hp -= physics.frameDuration * sparkingParams.damageFactor;
						if (hp <= 0.0f)
							kill = true;
					}
				}
				if (kill)
				{
					Tools::CreateAndPlaySound(killSoundBufferId, enemyActor.getOrigin2D(), [&](auto& sound) {
						const float basePitch = 5.0f * enemyGhostParams.initRadius / radius;
						sound.setPitch(glm::linearRand(basePitch, basePitch * 5.0f));
						sound.setVolume(0.2f);
						});
					for (auto& spark: sparks)
						spark->state = ComponentState::Outdated;
					enemyIdsToData.erase(enemyId);

					const float newRadius = radius * enemyGhostParams.radiusReductionFactor;
					if (newRadius >= enemyGhostParams.minimalRadius)
						for (int i = 0; i < enemyGhostParams.killSpawns; ++i)
							enemySpawn(enemyActor.getOrigin2D() + glm::circularRand(0.1f), radius * enemyGhostParams.radiusReductionFactor);
				}
			};
		}

		void enemyBoost(auto& enemy, glm::vec2 direction, float distance)
		{
			if (distance > 0.0f && distance < enemyGhostParams.boostDistance)
				enemy.setVelocity(direction / distance * enemyGhostParams.baseVelocity * enemyGhostParams.boostFactor);
		}

		bool sparkHandler(const auto& sourceData, auto& targetData, auto& spark, glm::vec2 direction, float distance, glm::vec4& damageColorFactor, bool fire)
		{
			if (distance <= sparkingParams.distance && fire && !sparkingOverheated)
			{
				const glm::vec2 sourceScalingFactor = playerParams.radius * playerParams.presentation.radiusProportions * glm::vec2(sourceData.sideFactor, 1.0f);
				const glm::vec2 weaponOffset =
					
					glm::translate(glm::mat4(1.0f), glm::vec3(playerParams.presentation.translation, 0.0f) * glm::vec3(sourceData.sideFactor, 1.0f, 1.0f))
					* glm::rotate(glm::mat4(1.0f), playerParams.presentation.rotation * sourceData.sideFactor, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::scale(glm::mat4(1.0f), glm::vec3(playerParams.presentation.scale * glm::vec2(sourceData.sideFactor, 1.0f), 1.0f))
					* glm::vec4(playerParams.presentation.weaponOffset, 0.0f, 1.0f);

				targetData.actor.setVelocity(direction / distance * enemyGhostParams.baseVelocity * enemyGhostParams.slowFactor);

				spark.vertices = Tools::Shapes2D::CreateVerticesOfLightning(sourceData.actor.getOrigin2D() + weaponOffset + glm::diskRand(glm::min(glm::abs(sourceScalingFactor.x), glm::abs(sourceScalingFactor.y)) * 0.1f),
					targetData.actor.getOrigin2D() + glm::diskRand(enemyGhostParams.minimalRadius), int(20 * distance), 2.0f / glm::sqrt(distance));
				spark.drawMode = GL_LINE_STRIP;
				spark.colorF = damageColorFactor = glm::mix(glm::vec4(0.0f, glm::linearRand(0.2f, 0.6f), glm::linearRand(0.4f, 0.8f), 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), sparkingOverheating) * 0.6f;
				spark.renderF = true;
				spark.state = ComponentState::Changed;

				return true;
			}

			spark.renderF = false;
			damageColorFactor = glm::vec4(1.0f);

			return false;
		}

		void sparkingHandler(auto& sourceData, bool fire)
		{
			const auto& physics = Globals::Components().physics();
			auto& sparkingSound = Globals::Components().sounds()[sparkingSoundId];
			auto& overchargeSound = Globals::Components().sounds()[overchargeSoundId];

			if (fire && sourceData.activeSparks)
			{
				if (!sparkingSound.isPlaying())
					sparkingSound.play();
				sparkingSound.setPosition(sourceData.actor.getOrigin2D());
				sparkingOverheating += physics.frameDuration * sparkingParams.overheatingRate;
				if (sparkingOverheating >= 1.0f)
				{
					sparkingSound.pause();
					if (!overchargeSound.isPlaying())
						overchargeSound.play();
					sparkingOverheating = 1.0f;
					sparkingOverheated = true;
				}
			}
			else
			{
				sparkingSound.pause();
				sparkingOverheating -= physics.frameDuration * sparkingParams.coolingRate;
				if (sparkingOverheating <= 0.0f)
				{
					overchargeSound.stop();
					sparkingOverheating = 0.0f;
					sparkingOverheated = false;
				}
			}

			overchargeSound.setVolume(sparkingOverheating);
			overchargeSound.setPosition(sourceData.actor.getOrigin2D());

			sourceData.activeSparks = 0;
		}

		void loadParams()
		{
			std::ifstream file(paramsPath);
			if (!file.is_open())
				std::cout << "unable to open \"" << paramsPath << "\"" << std::endl;

			std::string key, value;
			std::unordered_map<std::string, std::string> params;
			while (file >> key >> value)
				params[key] = value;

			std::cout << params.size() << " params loaded:" << std::endl;
			for (const auto& [key, value] : params)
				std::cout << key << " " << value << std::endl;

			auto getParam = [&](const std::string& key) {
				try
				{
					return params.at(key);
				}
				catch (...)
				{
					throw std::runtime_error("Failed to get param: " + key);
				}
			};

			{
				gameParams.numOfPlayers = Tools::Stoi(getParam("game.numOfPlayers"));
				gameParams.globalVolume = Tools::Stof(getParam("game.globalVolume"));
				gameParams.musicVolume = Tools::Stof(getParam("game.musicVolume"));
				gameParams.mapHSize = Tools::Stof(getParam("game.mapHSize"));
			}

			{
				playerParams.startPosition = { Tools::Stof(getParam("player.startPosition.x")), Tools::Stof(getParam("player.startPosition.y")) };
				playerParams.radius = Tools::Stof(getParam("player.radius"));
				playerParams.maxVelocity = Tools::Stof(getParam("player.maxVelocity"));
				playerParams.density = Tools::Stof(getParam("player.density"));
				playerParams.linearDamping = Tools::Stof(getParam("player.linearDamping"));
				playerParams.dash = Tools::Stof(getParam("player.dash"));

				playerParams.presentation.radiusProportions = { Tools::Stof(getParam("player.presentation.radiusProportions.x")), Tools::Stof(getParam("player.presentation.radiusProportions.y")) };
				playerParams.presentation.translation = { Tools::Stof(getParam("player.presentation.translation.x")), Tools::Stof(getParam("player.presentation.translation.y")) };
				playerParams.presentation.rotation = Tools::Stof(getParam("player.presentation.rotation"));
				playerParams.presentation.scale = { Tools::Stof(getParam("player.presentation.scale.x")), Tools::Stof(getParam("player.presentation.scale.y")) };
				playerParams.presentation.velocityRotationFactor = Tools::Stof(getParam("player.presentation.velocityRotationFactor"));
				playerParams.presentation.weaponOffset = { Tools::Stof(getParam("player.presentation.weaponOffset.x")), Tools::Stof(getParam("player.presentation.weaponOffset.y")) };

				playerParams.animation.textureSize = { Tools::Stoi(getParam("player.animation.textureSize.x")), Tools::Stoi(getParam("player.animation.textureSize.y")) };
				playerParams.animation.framesGrid = { Tools::Stoi(getParam("player.animation.framesGrid.x")), Tools::Stoi(getParam("player.animation.framesGrid.y")) };
				playerParams.animation.leftTopFrameLeftTopCorner = { Tools::Stoi(getParam("player.animation.leftTopFrameLeftTopCorner.x")), Tools::Stoi(getParam("player.animation.leftTopFrameLeftTopCorner.y")) };
				playerParams.animation.rightTopFrameLeftEdge = Tools::Stoi(getParam("player.animation.rightTopFrameLeftEdge"));
				playerParams.animation.leftBottomFrameTopEdge = Tools::Stoi(getParam("player.animation.leftBottomFrameTopEdge"));
				playerParams.animation.frameSize = { Tools::Stoi(getParam("player.animation.frameSize.x")), Tools::Stoi(getParam("player.animation.frameSize.y")) };
				playerParams.animation.frameDuration = Tools::Stof(getParam("player.animation.frameDuration"));
				playerParams.animation.numOfFrames = Tools::Stoi(getParam("player.animation.numOfFrames"));
				playerParams.animation.startFrame = Tools::Stoi(getParam("player.animation.startFrame"));

				playerParams.animation.direction = getParam("player.animation.direction") == "Forward"
					? AnimationData::Direction::Forward
					: AnimationData::Direction::Backward;

				playerParams.animation.mode = getParam("player.animation.mode") == "Repeat"
					? AnimationData::Mode::Repeat
					: getParam("player.animation.mode") == "Pingpong"
					? AnimationData::Mode::Pingpong
					: AnimationData::Mode::StopOnLastFrame;

				playerParams.animation.textureLayout = getParam("player.animation.textureLayout") == "Horizontal"
					? AnimationData::TextureLayout::Horizontal
					: AnimationData::TextureLayout::Vertical;

				playerParams.animation.frameTranslation = { Tools::Stof(getParam("player.animation.frameTranslation.x")), Tools::Stof(getParam("player.animation.frameTranslation.y")) };
				playerParams.animation.frameRotation = Tools::Stof(getParam("player.animation.frameRotation"));
				playerParams.animation.frameScale = { Tools::Stof(getParam("player.animation.frameScale.x")), Tools::Stof(getParam("player.animation.frameScale.y")) };
				playerParams.animation.neutralFrame = Tools::Stoi(getParam("player.animation.neutralFrame"));
			}

			{
				sparkingParams.distance = Tools::Stof(getParam("sparking.distance"));
				sparkingParams.damageFactor = Tools::Stof(getParam("sparking.damageFactor"));
				sparkingParams.overheatingRate = Tools::Stof(getParam("sparking.overheatingRate"));
				sparkingParams.coolingRate = Tools::Stof(getParam("sparking.coolingRate"));
			}

			{
				enemyGhostParams.initCount = Tools::Stoi(getParam("enemy.ghost.initCount"));
				enemyGhostParams.initSpawnPosition = { Tools::Stof(getParam("enemy.ghost.initSpawnPosition.x")), Tools::Stof(getParam("enemy.ghost.initSpawnPosition.y")) };
				enemyGhostParams.initHP = Tools::Stof(getParam("enemy.ghost.initHP"));
				enemyGhostParams.initRadius = Tools::Stof(getParam("enemy.ghost.initRadius"));
				enemyGhostParams.density = Tools::Stof(getParam("enemy.ghost.density"));
				enemyGhostParams.baseVelocity = Tools::Stof(getParam("enemy.ghost.baseVelocity"));
				enemyGhostParams.boostDistance = Tools::Stof(getParam("enemy.ghost.boostDistance"));
				enemyGhostParams.boostFactor = Tools::Stof(getParam("enemy.ghost.boostFactor"));
				enemyGhostParams.slowFactor = Tools::Stof(getParam("enemy.ghost.slowFactor"));
				enemyGhostParams.radiusReductionFactor = Tools::Stof(getParam("enemy.ghost.radiusReductionFactor"));
				enemyGhostParams.minimalRadius = Tools::Stof(getParam("enemy.ghost.minimalRadius"));
				enemyGhostParams.killSpawns = Tools::Stoi(getParam("enemy.ghost.killSpawns"));
			}
		}

		void reload(bool loadParams = true)
		{
			if (loadParams)
				this->loadParams();

			std::vector<glm::vec2> playerPositions;

			if (!playerIdsToData.empty())
			{
				playerPositions.reserve(playerIdsToData.size());
				for (const auto& [playerId, playerData] : playerIdsToData)
					playerPositions.push_back(playerData.actor.getOrigin2D());
				playerIdsToData.clear();
			}

			for (int i = 0; i < gameParams.numOfPlayers; ++i)
				playerSpawn(i, i < (int)playerPositions.size() ? playerPositions[i] : playerParams.startPosition + glm::circularRand(0.1f));

			auto& audioListener = Globals::Components().audioListener();
			audioListener.setVolume(gameParams.globalVolume);

			auto& musics = Globals::Components().musics();
			musics[musicId].setVolume(gameParams.musicVolume);
		}

		const Tools::BodyParams defaultBodyParams = Tools::BodyParams{}
			.linearDamping(6.0f)
			.fixedRotation(true);

		struct PlayerData
		{
			PlayerData(Components::Actor& actor, Components::AnimatedTexture& animatedTexture) :
				actor(actor),
				animatedTexture(animatedTexture)
			{
			}

			PlayerData(PlayerData&& playerData) noexcept :
				actor(playerData.actor),
				animatedTexture(playerData.animatedTexture)
			{
				playerData.outdated = true;
			}

			~PlayerData()
			{
				if (outdated)
					return;

				actor.state = ComponentState::Outdated;
				animatedTexture.state = ComponentState::Outdated;
			}

			Components::Actor& actor;
			Components::AnimatedTexture& animatedTexture;

			bool fire = false;
			bool autoFire = false;

			float sideFactor = 1.0f;
			int activeSparks{};
			float angle{};

			bool presentationTransparency = true;
			bool bodyRendering = false;

			bool outdated{};
		};

		struct EnemyData
		{
			EnemyData(Components::Actor& actor) :
				actor(actor)
			{
			}

			EnemyData(EnemyData&& enemyData) noexcept :
				actor(enemyData.actor)
			{
				enemyData.outdated = true;
			}

			~EnemyData()
			{
				if (outdated)
					return;

				actor.state = ComponentState::Outdated;
			}

			Components::Actor& actor;
			
			bool outdated{};
		};

		std::unordered_map<int, PlayerData> playerIdsToData;
		std::unordered_map<int, EnemyData> enemyIdsToData;

		ComponentId backgroundTextureId{};
		ComponentId coffinTextureId{};
		ComponentId fogTextureId{};

		ComponentId playerAnimationTextureId{};
		ComponentId enemyAnimationTextureId{};
		std::array<ComponentId, 10> enemyAnimatedTextureIds{};

		ComponentId musicId{};

		ComponentId sparkingSoundId{};
		ComponentId overchargeSoundId{};
		ComponentId dashSoundId{};

		ComponentId killSoundBufferId{};

		float sparkingOverheating = 0.0f;
		bool sparkingOverheated = false;

		GameParams gameParams{};
		PlayerParams playerParams{};
		SparkingParams sparkingParams{};
		EnemyGhostParams enemyGhostParams{};

		IdGenerator<int> enemyIdGenerator{};
	};

	AnimationTesting::AnimationTesting():
		impl(std::make_unique<Impl>())
	{
		impl->setup();
	}

	AnimationTesting::~AnimationTesting() = default;

	void AnimationTesting::postSetup()
	{
		impl->postSetup();
	}

	void AnimationTesting::step()
	{
		impl->step();
	}
}
