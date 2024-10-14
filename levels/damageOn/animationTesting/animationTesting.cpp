#include "animationTesting.hpp"

#include <components/graphicsSettings.hpp>
#include <components/screenInfo.hpp>
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
#include <format>

namespace Levels::DamageOn
{
	struct GameParams
	{
		int numOfPlayers;
		float globalVolume;
		float musicVolume;
		float mapHSize;

		struct Camera
		{
			float minProjectionHSize;
			float trackingMargin;
			float positionTransitionFactor;
			float projectionTransitionFactor;
		} camera;

		struct Gamepad
		{
			bool firstPlayer;
			float deadZone;
			float triggerDeadZone;
		} gamepad;
	};

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
	};

	struct ActorPresentation
	{
		glm::vec2 radiusProportions;
		glm::vec2 translation;
		float rotation;
		glm::vec2 scale;
		float velocityRotationFactor;
		float velocityScalingFactor;
		glm::vec2 weaponOffset;
	};

	struct Player
	{
		struct Params
		{
			glm::vec2 startPosition;
			float radius;
			float maxVelocity;
			float density;
			float linearDamping;
			float dash;

			ActorPresentation presentation;
			Animation animation;
		} params;

		struct Data
		{
			Data(Components::Actor& actor, Components::AnimatedTexture& animatedTexture, Components::Sound& sparkingSound, Components::Sound& overchargeSound, Components::Sound& dashSound) :
				actor(actor),
				animatedTexture(animatedTexture),
				sparkingSound(sparkingSound),
				overchargeSound(overchargeSound),
				dashSound(dashSound)
			{
			}

			Data(Data&& playerData) noexcept :
				actor(playerData.actor),
				animatedTexture(playerData.animatedTexture),
				sparkingSound(playerData.sparkingSound),
				overchargeSound(playerData.overchargeSound),
				dashSound(playerData.dashSound)
			{
				playerData.outdated = true;
			}

			~Data()
			{
				if (outdated)
					return;

				actor.state = ComponentState::Outdated;
				animatedTexture.state = ComponentState::Outdated;
				sparkingSound.state = ComponentState::Outdated;
				overchargeSound.state = ComponentState::Outdated;
				dashSound.state = ComponentState::Outdated;
			}

			Components::Actor& actor;
			Components::AnimatedTexture& animatedTexture;
			Components::Sound& sparkingSound;
			Components::Sound& overchargeSound;
			Components::Sound& dashSound;

			bool fire = false;
			bool autoFire = false;

			float manaOverheating = 0.0f;
			bool manaOverheated = false;

			float sideFactor = 1.0f;
			int activeSparks{};
			float angle{};

			bool outdated{};
		};
		std::unordered_map<int, Data> idsToData;
	};

	struct Enemy
	{
		struct Params
		{
			int initCount;
			glm::vec2 startPosition;
			float initHP;
			glm::vec2 initRadiusRange;
			float density;
			float baseVelocity;
			float boostDistance;
			float boostFactor;
			float slowFactor;
			float radiusReductionFactor;
			float minimalRadius;
			int killSpawns;

			ActorPresentation presentation;
			Animation animation;
		} params;

		struct Data
		{
			Data(Components::Actor& actor, Components::AnimatedTexture& animatedTexture) :
				actor(actor),
				animatedTexture(animatedTexture)
			{
			}

			Data(Data&& enemyData) noexcept :
				actor(enemyData.actor),
				animatedTexture(enemyData.animatedTexture)
			{
				enemyData.outdated = true;
			}

			~Data()
			{
				if (outdated)
					return;

				actor.state = ComponentState::Outdated;
				animatedTexture.state = ComponentState::Outdated;
			}

			Components::Actor& actor;
			Components::AnimatedTexture& animatedTexture;

			float sideFactor = 1.0f;
			float radius{};
			float angle{};

			bool outdated{};
		};
		std::unordered_map<int, Data> idsToData;
	};

	struct SparkingParams
	{
		float distance;
		float damageFactor;
		float overheatingRate;
		float coolingRate;
	};

	namespace
	{
		constexpr const char* paramsPath = "levels/damageOn/animationTesting/params.txt";

		constexpr int debrisCount = 20;
		constexpr float debrisDensity = 20.0f;
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

			backgroundTextureId = textures.emplace("textures/damageOn/egg.jpg", GL_CLAMP_TO_BORDER).getComponentId();
			//textures.last().magFilter = GL_NEAREST;
			textures.last().scale = glm::vec2(1.0f);
			//textures.last().preserveAspectRatio = true;

			coffinTextureId = textures.emplace("textures/damageOn/coffin.png", GL_MIRRORED_REPEAT).getComponentId();
			//textures.last().magFilter = GL_NEAREST;
			//textures.last().scale = glm::vec2(30.0f);

			fogTextureId = textures.size();
			textures.emplace("textures/damageOn/fog.png", GL_REPEAT);
			textures.last().scale = glm::vec2(0.15f);

			playerAnimationTextures.push_back(&textures.emplace("textures/damageOn/player 1.png"));
			textures.last().magFilter = GL_LINEAR;

			enemyAnimationTextures.push_back(&textures.emplace(TextureFile("textures/damageOn/enemy 1.jpg", 4, true, TextureFile::AdditionalConversion::DarkToTransparent, [](float* data, glm::ivec2 size, int numOfChannels) {
				for (int i = 0; i < size.x * size.y; ++i)
				{
					glm::vec4& pixel = reinterpret_cast<glm::vec4*>(data)[i];
					if (pixel.r + pixel.g + pixel.b < 0.2f)
						pixel = {};
				}
				})));
			textures.last().minFilter = GL_LINEAR;

			enemyAnimationTextures.push_back(&textures.emplace("textures/damageOn/enemy 2.png"));
			textures.last().magFilter = GL_LINEAR;

			enemyAnimationTextures.push_back(&textures.emplace("textures/damageOn/enemy 3.png"));
			textures.last().magFilter = GL_LINEAR;

			auto& soundsBuffers = Globals::Components().soundsBuffers();

			sparkingSoundBufferId = soundsBuffers.emplace("audio/Ghosthack Synth - Choatic_C.wav").getComponentId();
			overchargeSoundBufferId = soundsBuffers.emplace("audio/Ghosthack Scrape - Horror_C.wav").getComponentId();
			dashSoundBufferId = soundsBuffers.emplace("audio/Ghosthack Whoosh - 5.wav").getComponentId();
			killSoundBufferId = soundsBuffers.emplace("audio/Ghosthack Impact - Edge.wav").getComponentId();

			Tools::CreateFogForeground(5, 0.05f, fogTextureId, glm::vec4(1.0f), [x = 0.0f](int layer) mutable {
				(void)layer;
				const auto& physics = Globals::Components().physics();
				x += physics.frameDuration * 0.01f + layer * 0.00001f;
				const float y = std::sin(x * 30) * 0.01f;
				return glm::vec2(x, y);
			});

			auto& musics = Globals::Components().musics();
			musics.emplace("audio/DamageOn 2.ogg").play();
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
			const auto& screenInfo = Globals::Components().screenInfo();
			auto& camera = Globals::Components().camera2D();
			auto& walls = Globals::Components().staticWalls();
			auto& decorations = Globals::Components().staticDecorations();
			auto& textures = Globals::Components().staticTextures();
			auto& physics = Globals::Components().physics();

			const glm::vec2 levelHSize(textures[backgroundTextureId].loaded.getAspectRatio() * gameParams.mapHSize, gameParams.mapHSize);

			camera.positionTransitionFactor = gameParams.camera.positionTransitionFactor;
			camera.projectionTransitionFactor = gameParams.camera.projectionTransitionFactor;
			camera.targetPositionAndProjectionHSizeF = [&, levelHSize]() {
				glm::vec2 minPos(std::numeric_limits<float>::max());
				glm::vec2 maxPos(std::numeric_limits<float>::lowest());

				for (const auto& [playerId, playerData] : playerFrankenstein.idsToData)
				{
					const auto pos = playerData.actor.getOrigin2D();
					minPos.x = glm::min(minPos.x, pos.x);
					minPos.y = glm::min(minPos.y, pos.y);
					maxPos.x = glm::max(maxPos.x, pos.x);
					maxPos.y = glm::max(maxPos.y, pos.y);
				}

				glm::vec2 centerPos = (minPos + maxPos) * 0.5f;
				const float projectionHSize = glm::min(std::max(gameParams.camera.minProjectionHSize, glm::max((maxPos.x - minPos.x) / screenInfo.getAspectRatio(), maxPos.y - minPos.y) * 0.5f)
					+ gameParams.camera.trackingMargin, glm::min(levelHSize.x, levelHSize.y));

				if (centerPos.x - projectionHSize * screenInfo.getAspectRatio() < -levelHSize.x)
					centerPos.x = -levelHSize.x + projectionHSize * screenInfo.getAspectRatio();
				else if (centerPos.x + projectionHSize * screenInfo.getAspectRatio() > levelHSize.x)
					centerPos.x = levelHSize.x - projectionHSize * screenInfo.getAspectRatio();

				if (centerPos.y - projectionHSize < -levelHSize.y)
					centerPos.y = -levelHSize.y + projectionHSize;
				else if (centerPos.y + projectionHSize > levelHSize.y)
					centerPos.y = levelHSize.y - projectionHSize;

				return glm::vec3(centerPos, projectionHSize);
			};

			const float borderHThickness = 10.0f;
			walls.emplace(Tools::CreateBoxBody({ levelHSize.x + borderHThickness, borderHThickness }, Tools::BodyParams{}.position({ 0.0f, -levelHSize.y - borderHThickness }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			walls.emplace(Tools::CreateBoxBody({ levelHSize.x + borderHThickness, borderHThickness }, Tools::BodyParams{}.position({ 0.0f, levelHSize.y + borderHThickness }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			walls.emplace(Tools::CreateBoxBody({ borderHThickness, levelHSize.y + borderHThickness }, Tools::BodyParams{}.position({ -levelHSize.x - borderHThickness, 0.0f }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			walls.emplace(Tools::CreateBoxBody({ borderHThickness, levelHSize.y + borderHThickness }, Tools::BodyParams{}.position({ levelHSize.x + borderHThickness, 0.0f }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			const float mapScaleFactor = gameParams.mapHSize / 20.0f;
			const glm::vec2 eggCenter = glm::vec2(1.8f, 2.6f) * mapScaleFactor;
			walls.emplace(Tools::CreateCircleBody(6.0f * mapScaleFactor, Tools::BodyParams{}.position(eggCenter))).renderF = [&]() { return debug.levelBodiesRendering; };
			walls.last().colorF = glm::vec4(0.2f);
			walls.emplace(Tools::CreateCircleBody(5.5f * mapScaleFactor, Tools::BodyParams{}.position(eggCenter + glm::vec2(-2.5f) * mapScaleFactor))).renderF = [&]() { return debug.levelBodiesRendering; };
			walls.last().colorF = glm::vec4(0.2f);
			walls.emplace(Tools::CreateCircleBody(3.0f * mapScaleFactor, Tools::BodyParams{}.position(eggCenter + glm::vec2(-6.0f) * mapScaleFactor))).renderF = [&]() { return debug.levelBodiesRendering; };
			walls.last().colorF = glm::vec4(0.2f);

			auto screenCordTexturesF = [sceneCoordTextures = UniformsUtils::Uniform1b()](ShadersUtils::ProgramId program) mutable {
				if (!sceneCoordTextures.isValid())
					sceneCoordTextures = UniformsUtils::Uniform1b(program, "sceneCoordTextures");
				sceneCoordTextures(true);
				return [=]() mutable { sceneCoordTextures(false); };
			};

			for (int i = 0; i < debrisCount; ++i)
			{
				const float debrisWidth = glm::linearRand(0.3f, 1.0f);
				const float debrisHeight = debrisWidth * glm::linearRand(1.6f, 2.0f);
				walls.emplace(Tools::CreateBoxBody({ debrisWidth, debrisHeight }, Tools::BodyParams{}.position(glm::linearRand(-levelHSize, levelHSize)).angle(glm::linearRand(0.0f, glm::two_pi<float>()))
					.bodyType(b2_dynamicBody).linearDamping(10.0f).angularDamping(10.0f).density(debrisDensity)), CM::StaticTexture(coffinTextureId));
				walls.last().texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
			}

			decorations.emplace(Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, levelHSize), CM::StaticTexture(backgroundTextureId), Tools::Shapes2D::CreateTexCoordOfRectangle()).renderLayer = RenderLayer::FarBackground;
		}

		void step()
		{
			auto& keyboard = Globals::Components().keyboard();
			auto& physics = Globals::Components().physics();

			const bool anyGamepadStartPressed = []() {
				for (const auto& gamepad : Globals::Components().gamepads())
					if (gamepad.pressed.start)
						return true;
				return false;
			}();

			if (keyboard.pressed['P'] || anyGamepadStartPressed)
				reload();

			std::unordered_map<int, float> minDistances;
			for (auto& [playerId, playerData] : playerFrankenstein.idsToData)
			{
				const bool keyboardEnabled = gameParams.gamepad.firstPlayer ? true : playerId == 0;
				bool gamepadEnabled = gameParams.gamepad.firstPlayer ? true : playerId > 0;
				int gamepadId = gamepadEnabled ? (playerId - !gameParams.gamepad.firstPlayer) : 0;
				auto& gamepad = Globals::Components().gamepads()[gamepadId];

				if (gamepadId >= (int)Globals::Components().gamepads().size())
				{
					gamepadId = 0;
					gamepadEnabled = false;
				}

				auto enemyInteractionsF = [&](auto& enemy) {
					for (auto& [enemyId, enemyData] : enemy.idsToData)
					{
						const auto direction = playerData.actor.getOrigin2D() - enemyData.actor.getOrigin2D();
						const auto distance = glm::length(direction);
						if (auto it = minDistances.find(enemyId); it == minDistances.end() || (distance > 0.0f && distance < it->second))
						{
							enemyData.actor.setVelocity(direction / distance * enemy.params.baseVelocity);
							enemyBoost(enemy, enemyData.actor, direction, distance);
							enemyData.sideFactor = direction.x < 0.0f ? -1.0f : 1.0f;
							const float vLength = glm::length(enemyData.actor.getVelocity());
							enemyData.angle = -glm::min(glm::quarter_pi<float>(), (vLength * vLength * enemy.params.presentation.velocityRotationFactor));
							enemyData.animatedTexture.setAdditionalTransformation(enemy.params.animation.frameTranslation * glm::vec2(enemyData.sideFactor, 1.0f),
								enemy.params.animation.frameRotation * enemyData.sideFactor, enemy.params.animation.frameScale * glm::vec2(enemyData.sideFactor, 1.0f));
							enemyData.animatedTexture.setSpeedScaling(enemy.params.presentation.velocityScalingFactor == 0.0f ? 1.0f : glm::length(enemyData.actor.getVelocity() * enemy.params.presentation.velocityScalingFactor));

							minDistances[enemyId] = distance;
						}
					}
				};
				enemyInteractionsF(enemyGhost);
				enemyInteractionsF(enemyChicken);
				enemyInteractionsF(enemyZombie);

				const glm::vec2 gamepadDirection = gamepadEnabled
					? [&]() {
						if (!gamepad.enabled)
							return glm::vec2(0.0f);
						const float directionLength = glm::length(gamepad.lStick);
						if (directionLength < gameParams.gamepad.deadZone)
							return glm::vec2(0.0f);
						if (directionLength > 1.0f)
							return gamepad.lStick / directionLength;
						return gamepad.lStick;
					}()
					: glm::vec2(0.0f);

				const glm::vec2 keyboardDirection = keyboardEnabled
					? [&]() {
						const glm::vec2 direction(-(int)keyboard.pressing[/*VK_LEFT*/0x25] + (int)keyboard.pressing[/*VK_RIGHT*/0x27], -(int)keyboard.pressing[/*VK_DOWN*/0x28] + (int)keyboard.pressing[/*VK_UP*/0x26]);
						if (direction == glm::vec2(0.0f))
							return glm::vec2(0.0f);
						return direction / glm::length(direction);
					}()
					: glm::vec2(0.0f);

				const glm::vec2 direction = [&]() {
					const glm::vec2 direction = gamepadDirection + keyboardDirection;
					if (glm::length(direction) > 1.0f)
						return direction / glm::length(direction);
					return direction;
				}();

				const float vLength = glm::length(playerData.actor.getVelocity());

				playerData.angle = -glm::min(glm::quarter_pi<float>(), (vLength * vLength * playerFrankenstein.params.presentation.velocityRotationFactor));

				playerData.animatedTexture.setSpeedScaling(playerFrankenstein.params.presentation.velocityScalingFactor == 0.0f ? 1.0f : vLength * playerFrankenstein.params.presentation.velocityScalingFactor);
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
					playerData.animatedTexture.forceFrame(playerFrankenstein.params.animation.neutralFrame);

				playerData.animatedTexture.setAdditionalTransformation(playerFrankenstein.params.animation.frameTranslation * glm::vec2(playerData.sideFactor, 1.0f),
					playerFrankenstein.params.animation.frameRotation * playerData.sideFactor, playerFrankenstein.params.animation.frameScale * glm::vec2(playerData.sideFactor, 1.0f));

				const glm::vec2 newVelocity = direction * playerFrankenstein.params.maxVelocity;
				if (glm::length(newVelocity) > glm::length(playerData.actor.getVelocity()))
					playerData.actor.setVelocity(newVelocity);

				if (keyboard.pressing[/*VK_CONTROL*/0x11] * keyboardEnabled || gamepad.rTrigger * gamepadEnabled > gameParams.gamepad.triggerDeadZone || gamepad.lTrigger * gamepadEnabled > gameParams.gamepad.triggerDeadZone)
				{
					playerData.fire = true;
					playerData.autoFire = false;
				}
				else
					playerData.fire = false;

				if (keyboard.pressed[/*VK_SHIFT*/0x10] * keyboardEnabled || gamepad.pressed.rShoulder * gamepadEnabled || gamepad.pressed.lShoulder * gamepadEnabled)
					playerData.autoFire = !playerData.autoFire;

				if ((keyboard.pressed[/*VK_SPACE*/0x20] * keyboardEnabled || gamepad.pressed.a * gamepadEnabled) && glm::length(direction) > 0.0f)
				{
					playerData.dashSound.stop().setPlayingOffset(0.35f).setPosition(playerData.actor.getOrigin2D()).play();
					playerData.actor.body->ApplyLinearImpulseToCenter(ToVec2<b2Vec2>(direction * playerFrankenstein.params.dash * playerData.actor.body->GetMass()), true);
				}

				if (keyboard.pressed['T'] * keyboardEnabled || gamepad.pressed.y * gamepadEnabled)
					debug.presentationTransparency = !debug.presentationTransparency;
				if (keyboard.pressed['B'] * keyboardEnabled || gamepad.pressed.x * gamepadEnabled)
					debug.bodyRendering = !debug.bodyRendering;
				if (keyboard.pressed['L'] * keyboardEnabled)
					debug.levelBodiesRendering = !debug.levelBodiesRendering;

				sparkingHandler(playerData, playerData.fire || playerData.autoFire);
			}
		}

	private:
		void playerSpawn(Player& player, glm::vec2 position)
		{
			const auto& physics = Globals::Components().physics();
			auto& sounds = Globals::Components().sounds();

			auto& playerActor = Globals::Components().actors().emplace(Tools::CreateCircleBody(player.params.radius, Tools::BodyParams{}
				.linearDamping(player.params.linearDamping).fixedRotation(true).bodyType(b2_dynamicBody).density(player.params.density).position(position)), CM::DummyTexture{});

			auto& playerAnimatedTexture = Globals::Components().dynamicAnimatedTextures().emplace();
			playerAnimatedTexture.setAnimationData({ CM::StaticTexture(playerAnimationTextures[0]->getComponentId()), player.params.animation.textureSize, player.params.animation.framesGrid, player.params.animation.leftTopFrameLeftTopCorner,
				player.params.animation.rightTopFrameLeftEdge, player.params.animation.leftBottomFrameTopEdge, player.params.animation.frameSize, player.params.animation.frameDuration, player.params.animation.numOfFrames,
				player.params.animation.startFrame == -1 ? glm::linearRand(0, player.params.animation.numOfFrames - 1) : player.params.animation.startFrame,
				player.params.animation.direction, player.params.animation.mode, player.params.animation.textureLayout });
			playerAnimatedTexture.start(true);

			auto& sparkingSound = sounds.emplace(sparkingSoundBufferId);
			sparkingSound.setLoop(true).play().pause();
			auto& overchargingSound = sounds.emplace(overchargeSoundBufferId);
			overchargingSound.setLoop(true).play().pause();
			auto& dashSoundBuffer = sounds.emplace(dashSoundBufferId);
			dashSoundBuffer.setVolume(0.15f);

			const auto playerId = playerIdGenerator.acquire();
			const auto& playerData = player.idsToData.emplace(playerId, Player::Data{ playerActor, playerAnimatedTexture, sparkingSound, overchargingSound, dashSoundBuffer }).first->second;

			playerActor.renderF = [&]() { return debug.bodyRendering; };
			playerActor.colorF = glm::vec4(0.4f);
			playerActor.posInSubsequence = 1;

			const auto playerPresentationSize = player.params.presentation.radiusProportions * player.params.radius;
			auto& playerPresentation = playerActor.subsequence.emplace_back();
			playerPresentation.renderingSetupF = [&](auto) { if (!debug.presentationTransparency) glDisable(GL_BLEND); return []() mutable { glEnable(GL_BLEND); }; };
			playerPresentation.vertices = Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, playerPresentationSize);
			playerPresentation.texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
			playerPresentation.texture = CM::DynamicAnimatedTexture(playerData.animatedTexture.getComponentId());
			playerPresentation.modelMatrixF = [&]() {
				return playerActor.modelMatrixF() * glm::translate(glm::mat4(1.0f), glm::vec3(player.params.presentation.translation, 0.0f) * glm::vec3(playerData.sideFactor, 1.0f, 1.0f))
					* glm::rotate(glm::mat4(1.0f), (player.params.presentation.rotation + playerData.angle) * playerData.sideFactor, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::scale(glm::mat4(1.0f), glm::vec3(player.params.presentation.scale, 1.0f));
			};
			playerPresentation.colorF = [&]() {
				return glm::mix(glm::vec4(1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), playerData.manaOverheating) * (playerData.manaOverheated ? (glm::sin(physics.simulationDuration * 20.0f) + 1.0f) / 2.0f : 1.0f);
			};
		}

		void enemySpawn(Enemy& enemy, glm::vec2 position, float radius, int enemyType)
		{
			const auto& physics = Globals::Components().physics();

			auto& enemyAnimatedTexture = Globals::Components().dynamicAnimatedTextures().add({ CM::StaticTexture(enemyAnimationTextures[enemyType]->getComponentId()), enemy.params.animation.textureSize,
				enemy.params.animation.framesGrid, enemy.params.animation.leftTopFrameLeftTopCorner, enemy.params.animation.rightTopFrameLeftEdge, enemy.params.animation.leftBottomFrameTopEdge,
				enemy.params.animation.frameSize, enemy.params.animation.frameDuration, enemy.params.animation.numOfFrames,
				enemy.params.animation.startFrame == -1 ? glm::linearRand(0, enemy.params.animation.numOfFrames - 1) : enemy.params.animation.startFrame,
				enemy.params.animation.direction, enemy.params.animation.mode, enemy.params.animation.textureLayout });
			enemyAnimatedTexture.start(true);

			auto& enemyActor = Globals::Components().actors().emplace(Tools::CreateCircleBody(radius, Tools::BodyParams{ defaultBodyParams }
				.bodyType(b2_dynamicBody)
				.density(enemy.params.density)
				.position(position)), CM::DummyTexture());

			const auto enemyId = enemyIdGenerator.acquire();
			auto& enemyData = enemy.idsToData.emplace(enemyId, Enemy::Data{ enemyActor, enemyAnimatedTexture }).first->second;
			enemyData.radius = radius;

			enemyActor.renderF = [&]() { return debug.bodyRendering; };
			enemyActor.colorF = glm::vec4(0.4f);
			enemyActor.posInSubsequence = 1;

			auto& enemyPresentation = enemyActor.subsequence.emplace_back();
			const auto enemyPresentationSize = enemy.params.presentation.radiusProportions * radius;

			enemyPresentation.vertices = Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, enemyPresentationSize);
			enemyPresentation.modelMatrixF = [&]() {
				return enemyActor.modelMatrixF() * glm::translate(glm::mat4(1.0f), glm::vec3(enemy.params.presentation.translation, 0.0f) * glm::vec3(enemyData.sideFactor, 1.0f, 1.0f))
					* glm::rotate(glm::mat4(1.0f), (enemy.params.presentation.rotation + enemyData.angle) * enemyData.sideFactor, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::scale(glm::mat4(1.0f), glm::vec3(enemy.params.presentation.scale, 1.0f));
				};

			auto damageColorFactor = std::make_shared<glm::vec4>(1.0f);
			if (enemyType == 0)
			{
				enemyPresentation.texture = CM::DynamicAnimatedTexture(enemyData.animatedTexture.getComponentId(), {}, {}, glm::vec2(2.6f * radius));
				enemyPresentation.renderingSetupF = createRecursiveFaceRS({ radius * 0.6f, radius });
				enemyPresentation.colorF = [baseColor = glm::vec4(glm::vec3(glm::linearRand(0.0f, 1.0f)), 1.0f) * 0.8f, damageColorFactor]() { return glm::mix(baseColor, *damageColorFactor, 0.5f); };
			}
			else
			{
				enemyPresentation.texture = CM::DynamicAnimatedTexture(enemyData.animatedTexture.getComponentId());
				enemyPresentation.texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
				enemyPresentation.renderingSetupF = [&](auto) { if (!debug.presentationTransparency) glDisable(GL_BLEND); return []() mutable { glEnable(GL_BLEND); }; };
				enemyPresentation.colorF = [
					baseColor = glm::vec4(glm::vec3(glm::linearRand(0.5f, 1.0f), glm::linearRand(0.5f, 1.0f), glm::linearRand(0.5f, 1.0f)), 1.0f),
						damageColorFactor
				]() { return glm::mix(baseColor, *damageColorFactor, 0.5f); };
			}

			enemyActor.stepF = [&, enemyId, radius, enemyType, sparks = std::vector<Components::Decoration*>{}, damageColorFactor, hp = enemy.params.initHP]() mutable {
				if (sparks.size() != gameParams.numOfPlayers)
				{
					for (auto& spark: sparks)
						spark->state = ComponentState::Outdated;
					sparks.clear();
					sparks.reserve(gameParams.numOfPlayers);
					for (int i = 0; i < gameParams.numOfPlayers; ++i)
						sparks.push_back(&Globals::Components().dynamicDecorations().emplace());
				}

				bool kill = false;
				for (auto& [playerId, playerData] : playerFrankenstein.idsToData)
				{
					const auto direction = playerData.actor.getOrigin2D() - enemyActor.getOrigin2D();
					const auto distance = glm::length(direction);

					if (sparkHandler(playerData, enemy, enemyData, *sparks[playerId], direction, distance, *damageColorFactor, playerData.fire || playerData.autoFire))
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
						const float basePitch = 5.0f * enemyData.radius / radius;
						sound.setPitch(glm::linearRand(basePitch, basePitch * 5.0f));
						sound.setVolume(0.2f);
						});
					for (auto& spark: sparks)
						spark->state = ComponentState::Outdated;
					enemy.idsToData.erase(enemyId);

					const float newRadius = radius * enemy.params.radiusReductionFactor;
					if (enemy.params.killSpawns > 0 && newRadius >= enemy.params.minimalRadius)
						for (int i = 0; i < enemy.params.killSpawns; ++i)
							enemySpawn(enemy, enemyActor.getOrigin2D() + glm::circularRand(0.1f), radius * enemy.params.radiusReductionFactor, enemyType);
				}
			};
		}

		void enemyBoost(const Enemy& enemy, auto& enemyBody, glm::vec2 direction, float distance)
		{
			if (distance > 0.0f && distance < enemy.params.boostDistance)
				enemyBody.setVelocity(direction / distance * enemy.params.baseVelocity * enemy.params.boostFactor);
		}

		bool sparkHandler(const auto& sourceData, const auto& target, auto& targetData, auto& spark, glm::vec2 direction, float distance, glm::vec4& damageColorFactor, bool fire)
		{
			if (distance <= sparkingParams.distance && fire && !sourceData.manaOverheated)
			{
				const glm::vec2 sourceScalingFactor = playerFrankenstein.params.radius * playerFrankenstein.params.presentation.radiusProportions * glm::vec2(sourceData.sideFactor, 1.0f);
				const glm::vec2 weaponOffset =
					glm::translate(glm::mat4(1.0f), glm::vec3(playerFrankenstein.params.presentation.translation, 0.0f) * glm::vec3(sourceData.sideFactor, 1.0f, 1.0f))
					* glm::rotate(glm::mat4(1.0f), playerFrankenstein.params.presentation.rotation * sourceData.sideFactor, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::scale(glm::mat4(1.0f), glm::vec3(playerFrankenstein.params.presentation.scale * glm::vec2(sourceData.sideFactor, 1.0f), 1.0f))
					* glm::vec4(playerFrankenstein.params.presentation.weaponOffset, 0.0f, 1.0f);

				targetData.actor.setVelocity(direction / distance * target.params.baseVelocity * target.params.slowFactor);

				spark.vertices = Tools::Shapes2D::CreateVerticesOfLightning(sourceData.actor.getOrigin2D() + weaponOffset + glm::diskRand(glm::min(glm::abs(sourceScalingFactor.x), glm::abs(sourceScalingFactor.y)) * 0.1f),
					targetData.actor.getOrigin2D() + glm::diskRand(target.params.minimalRadius), int(20 * distance), 2.0f / glm::sqrt(distance));
				spark.drawMode = GL_LINE_STRIP;
				spark.colorF = damageColorFactor = glm::mix(glm::vec4(0.0f, glm::linearRand(0.2f, 0.6f), glm::linearRand(0.4f, 0.8f), 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), sourceData.manaOverheating) * 0.6f;
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

			if (fire && sourceData.activeSparks)
			{
				if (!sourceData.sparkingSound.isPlaying())
					sourceData.sparkingSound.play();
				sourceData.sparkingSound.setPosition(sourceData.actor.getOrigin2D());
				sourceData.manaOverheating += physics.frameDuration * sparkingParams.overheatingRate;
				if (sourceData.manaOverheating >= 1.0f)
				{
					sourceData.sparkingSound.pause();
					if (!sourceData.overchargeSound.isPlaying())
						sourceData.overchargeSound.play();
					sourceData.manaOverheating = 1.0f;
					sourceData.manaOverheated = true;
				}
			}
			else
			{
				sourceData.sparkingSound.pause();
				sourceData.manaOverheating -= physics.frameDuration * sparkingParams.coolingRate;
				if (sourceData.manaOverheating <= 0.0f)
				{
					sourceData.overchargeSound.stop();
					sourceData.manaOverheating = 0.0f;
					sourceData.manaOverheated = false;
				}
			}

			sourceData.overchargeSound.setVolume(sourceData.manaOverheating);
			sourceData.overchargeSound.setPosition(sourceData.actor.getOrigin2D());

			sourceData.activeSparks = 0;
		}

		void loadParams()
		{
			std::ifstream file(paramsPath);
			if (!file.is_open())
				std::cout << "unable to open \"" << paramsPath << "\"" << std::endl;

			std::unordered_map<std::string, std::string> params;
			std::string line;
			while (std::getline(file, line)) {
				std::istringstream iss(line);
				std::string key, value;

				if (iss >> key) {
					std::getline(iss, value);
					value.erase(0, 1);
					params.emplace(std::move(key), std::move(value));
				}
			}

			std::cout << params.size() << " params loaded:" << std::endl;
			for (const auto& [key, value] : params)
				std::cout << key << " " << value << std::endl;

			auto getValue = [&](const std::string& key) {
				try
				{
					return params.at(key);
				}
				catch (...)
				{
					throw std::runtime_error("Failed to get value for key " + key);
				}
			};

			auto loadParam = [&](auto& param, const std::string& key) {
				using ParamType = std::remove_reference_t<decltype(param)>;

				if constexpr (std::is_same_v<ParamType, int>)
					param = Tools::Stoi(getValue(key));
				else if constexpr (std::is_same_v<ParamType, float>)
					param = Tools::Stof(getValue(key));
				else if constexpr (std::is_same_v<ParamType, bool>)
					param = getValue(key) == "true";
				else if constexpr (std::is_same_v<ParamType, std::string>)
					param = getValue(key);
				else if constexpr (std::is_same_v<ParamType, glm::vec2>)
				{
					const auto value = getValue(key);
					const auto spacePos = value.find(' ');
					param = { Tools::Stof(value.substr(0, spacePos)), Tools::Stof(value.substr(spacePos + 1)) };
				}
				else if constexpr (std::is_same_v<ParamType, glm::ivec2>)
				{
					const auto value = getValue(key);
					const auto spacePos = value.find(' ');
					param = { Tools::Stoi(value.substr(0, spacePos)), Tools::Stoi(value.substr(spacePos + 1)) };
				}
				else if constexpr (std::is_same_v<ParamType, glm::vec3>)
				{
					const auto value = getValue(key);
					const auto spacePos1 = value.find(' ');
					const auto spacePos2 = value.find(' ', spacePos1 + 1);
					param = { Tools::Stof(value.substr(0, spacePos1)), Tools::Stof(value.substr(spacePos1 + 1, spacePos2 - spacePos1 - 1)), Tools::Stof(value.substr(spacePos2 + 1)) };
				}
				else if constexpr (std::is_same_v<ParamType, glm::ivec3>)
				{
					const auto value = getValue(key);
					const auto spacePos1 = value.find(' ');
					const auto spacePos2 = value.find(' ', spacePos1 + 1);
					param = { Tools::Stoi(value.substr(0, spacePos1)), Tools::Stoi(value.substr(spacePos1 + 1, spacePos2 - spacePos1 - 1)), Tools::Stoi(value.substr(spacePos2 + 1)) };
				}
				else if constexpr (std::is_same_v<ParamType, glm::vec4>)
				{
					const auto value = getValue(key);
					const auto spacePos1 = value.find(' ');
					const auto spacePos2 = value.find(' ', spacePos1 + 1);
					const auto spacePos3 = value.find(' ', spacePos2 + 1);
					param = { Tools::Stof(value.substr(0, spacePos1)), Tools::Stof(value.substr(spacePos1 + 1, spacePos2 - spacePos1 - 1)),
						Tools::Stof(value.substr(spacePos2 + 1, spacePos3 - spacePos2 - 1)), Tools::Stof(value.substr(spacePos3 + 1)) };
				}
				else if constexpr (std::is_same_v<ParamType, glm::ivec4>)
				{
					const auto value = getValue(key);
					const auto spacePos1 = value.find(' ');
					const auto spacePos2 = value.find(' ', spacePos1 + 1);
					const auto spacePos3 = value.find(' ', spacePos2 + 1);
					param = { Tools::Stoi(value.substr(0, spacePos1)), Tools::Stoi(value.substr(spacePos1 + 1, spacePos2 - spacePos1 - 1)),
						Tools::Stoi(value.substr(spacePos2 + 1, spacePos3 - spacePos2 - 1)), Tools::Stoi(value.substr(spacePos3 + 1)) };
				}
				else if constexpr (std::is_same_v<ParamType, AnimationData::Direction>)
					param = getValue(key) == "forward" ? AnimationData::Direction::Forward : AnimationData::Direction::Backward;
				else if constexpr (std::is_same_v<ParamType, AnimationData::Mode>)
					param = getValue(key) == "repeat" ? AnimationData::Mode::Repeat : getValue(key) == "pingpong" ? AnimationData::Mode::Pingpong : AnimationData::Mode::StopOnLastFrame;
				else if constexpr (std::is_same_v<ParamType, AnimationData::TextureLayout>)
					param = getValue(key) == "horizontal" ? AnimationData::TextureLayout::Horizontal : AnimationData::TextureLayout::Vertical;
				else
					throw std::runtime_error("Unsupported type for key " + key);
			};

			auto loadPresentationParams = [&](auto& presentationParams, std::string actorName) {
				loadParam(presentationParams.radiusProportions, std::format("{}.presentation.radiusProportions", actorName));
				loadParam(presentationParams.translation, std::format("{}.presentation.translation", actorName));
				loadParam(presentationParams.rotation, std::format("{}.presentation.rotation", actorName));
				loadParam(presentationParams.scale, std::format("{}.presentation.scale", actorName));
				loadParam(presentationParams.velocityRotationFactor, std::format("{}.presentation.velocityRotationFactor", actorName));
				loadParam(presentationParams.velocityScalingFactor, std::format("{}.presentation.velocityScalingFactor", actorName));
				loadParam(presentationParams.weaponOffset, std::format("{}.presentation.weaponOffset", actorName));
			};

			auto loadAnimationParams = [&](auto& animationParams, std::string actorName) {
				loadParam(animationParams.textureSize, std::format("{}.animation.textureSize", actorName));
				loadParam(animationParams.framesGrid, std::format("{}.animation.framesGrid", actorName));
				loadParam(animationParams.leftTopFrameLeftTopCorner, std::format("{}.animation.leftTopFrameLeftTopCorner", actorName));
				loadParam(animationParams.rightTopFrameLeftEdge, std::format("{}.animation.rightTopFrameLeftEdge", actorName));
				loadParam(animationParams.leftBottomFrameTopEdge, std::format("{}.animation.leftBottomFrameTopEdge", actorName));
				loadParam(animationParams.frameSize, std::format("{}.animation.frameSize", actorName));
				loadParam(animationParams.frameDuration, std::format("{}.animation.frameDuration", actorName));
				loadParam(animationParams.numOfFrames, std::format("{}.animation.numOfFrames", actorName));
				loadParam(animationParams.startFrame, std::format("{}.animation.startFrame", actorName));
				loadParam(animationParams.direction, std::format("{}.animation.direction", actorName));
				loadParam(animationParams.mode, std::format("{}.animation.mode", actorName));
				loadParam(animationParams.textureLayout, std::format("{}.animation.textureLayout", actorName));
				loadParam(animationParams.frameTranslation, std::format("{}.animation.frameTranslation", actorName));
				loadParam(animationParams.frameRotation, std::format("{}.animation.frameRotation", actorName));
				loadParam(animationParams.frameScale, std::format("{}.animation.frameScale", actorName));
				loadParam(animationParams.neutralFrame, std::format("{}.animation.neutralFrame", actorName));
			};

			auto loadPlayerParams = [&](auto& playerParams, std::string playerName) {
				loadParam(playerParams.startPosition, std::format("player.{}.startPosition", playerName));
				loadParam(playerParams.radius, std::format("player.{}.radius", playerName));
				loadParam(playerParams.maxVelocity, std::format("player.{}.maxVelocity", playerName));
				loadParam(playerParams.density, std::format("player.{}.density", playerName));
				loadParam(playerParams.linearDamping, std::format("player.{}.linearDamping", playerName));
				loadParam(playerParams.dash, std::format("player.{}.dash", playerName));
				loadPresentationParams(playerParams.presentation, std::format("player.{}", playerName));
				loadAnimationParams(playerParams.animation, std::format("player.{}", playerName));
			};

			auto loadEnemyParams = [&](auto& enemyParams, std::string enemyName) {
				loadParam(enemyParams.initCount, std::format("enemy.{}.initCount", enemyName));
				loadParam(enemyParams.startPosition, std::format("enemy.{}.startPosition", enemyName));
				loadParam(enemyParams.initHP, std::format("enemy.{}.initHP", enemyName));
				loadParam(enemyParams.initRadiusRange, std::format("enemy.{}.initRadiusRange", enemyName));
				loadParam(enemyParams.density, std::format("enemy.{}.density", enemyName));
				loadParam(enemyParams.baseVelocity, std::format("enemy.{}.baseVelocity", enemyName));
				loadParam(enemyParams.boostDistance, std::format("enemy.{}.boostDistance", enemyName));
				loadParam(enemyParams.boostFactor, std::format("enemy.{}.boostFactor", enemyName));
				loadParam(enemyParams.slowFactor, std::format("enemy.{}.slowFactor", enemyName));
				loadParam(enemyParams.radiusReductionFactor, std::format("enemy.{}.radiusReductionFactor", enemyName));
				loadParam(enemyParams.minimalRadius, std::format("enemy.{}.minimalRadius", enemyName));
				loadParam(enemyParams.killSpawns, std::format("enemy.{}.killSpawns", enemyName));
				loadPresentationParams(enemyParams.presentation, std::format("enemy.{}", enemyName));
				loadAnimationParams(enemyParams.animation, std::format("enemy.{}", enemyName));
			};

			loadParam(gameParams.numOfPlayers, "game.numOfPlayers");
			loadParam(gameParams.globalVolume, "game.globalVolume");
			loadParam(gameParams.musicVolume, "game.musicVolume");
			loadParam(gameParams.mapHSize, "game.mapHSize");
			loadParam(gameParams.camera.minProjectionHSize, "game.camera.minProjectionHSize");
			loadParam(gameParams.camera.trackingMargin, "game.camera.trackingMargin");
			loadParam(gameParams.camera.positionTransitionFactor, "game.camera.positionTransitionFactor");
			loadParam(gameParams.camera.projectionTransitionFactor, "game.camera.projectionTransitionFactor");
			loadParam(gameParams.gamepad.firstPlayer, "game.gamepad.firstPlayer");
			loadParam(gameParams.gamepad.deadZone, "game.gamepad.deadZone");
			loadParam(gameParams.gamepad.triggerDeadZone, "game.gamepad.triggerDeadZone");

			loadPlayerParams(playerFrankenstein.params, "frankenstein");

			loadEnemyParams(enemyGhost.params, "ghost");
			loadEnemyParams(enemyChicken.params, "chicken");
			loadEnemyParams(enemyZombie.params, "zombie");

			loadParam(sparkingParams.distance, "sparking.distance");
			loadParam(sparkingParams.damageFactor, "sparking.damageFactor");
			loadParam(sparkingParams.overheatingRate, "sparking.overheatingRate");
			loadParam(sparkingParams.coolingRate, "sparking.coolingRate");
		}

		void reload(bool loadParams = true)
		{
			if (loadParams)
				this->loadParams();

			auto clearPlayer = [&](auto& player) {
				std::vector<glm::vec2> playerPositions;
				if (!player.idsToData.empty())
				{
					playerPositions.reserve(player.idsToData.size());
					for (const auto& [playerId, playerData] : player.idsToData)
						playerPositions.push_back(playerData.actor.getOrigin2D());
					player.idsToData.clear();
				}
				return playerPositions;
			};

			auto clearEnemy = [&](auto& enemy) {
				std::vector<glm::vec3> enemyPositionsAndRadiuses;
				if (!enemy.idsToData.empty())
				{
					enemyPositionsAndRadiuses.reserve(enemy.idsToData.size());
					for (const auto& [enemyId, enemyData] : enemy.idsToData)
						enemyPositionsAndRadiuses.emplace_back(enemyData.actor.getOrigin2D(), enemyData.radius);
					enemy.idsToData.clear();
				}
				return enemyPositionsAndRadiuses;
			};

			auto spawnPlayer = [&](auto& player, const auto& playerPositions) {
				for (int i = 0; i < gameParams.numOfPlayers; ++i)
					playerSpawn(player, i < (int)playerPositions.size() ? playerPositions[i] : player.params.startPosition + glm::circularRand(0.1f));
			};

			auto spawnEnemy = [&](auto& enemy, const auto& enemyPositionsAndRadiuses, int enemyType) {
				for (int i = 0; i < enemy.params.initCount; ++i)
					if (i < (int)enemyPositionsAndRadiuses.size())
						enemySpawn(enemy, glm::vec2(enemyPositionsAndRadiuses[i]), enemyPositionsAndRadiuses[i].z, enemyType);
					else
						enemySpawn(enemy, enemy.params.startPosition + glm::circularRand(0.1f), glm::linearRand(enemy.params.initRadiusRange.x, enemy.params.initRadiusRange.y), enemyType);
			};

			playerIdGenerator.reset();
			enemyIdGenerator.reset();

			const auto playerFrankensteinPositions = clearPlayer(playerFrankenstein);
			const auto enemyGhostPositionsAndRadiuses = clearEnemy(enemyGhost);
			const auto enemyChickenPositionsAndRadiuses = clearEnemy(enemyChicken);
			const auto enemyZombiePositionsAndRadiuses = clearEnemy(enemyZombie);

			spawnPlayer(playerFrankenstein, playerFrankensteinPositions);
			spawnEnemy(enemyGhost, enemyGhostPositionsAndRadiuses, 0);
			spawnEnemy(enemyChicken, enemyChickenPositionsAndRadiuses, 1);
			spawnEnemy(enemyZombie, enemyZombiePositionsAndRadiuses, 2);

			auto& audioListener = Globals::Components().audioListener();
			audioListener.setVolume(gameParams.globalVolume);

			auto& musics = Globals::Components().musics();
			musics[musicId].setVolume(gameParams.musicVolume);
		}

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

				if (!debug.presentationTransparency)
					glDisable(GL_BLEND);

				visibilityReduction(true);
				fullVisibilityDistance(fadingRange.x);
				invisibilityDistance(fadingRange.y);

				return [=]() mutable {
					visibilityReduction(false);
					glEnable(GL_BLEND);
				};
			} };
		}

		const Tools::BodyParams defaultBodyParams = Tools::BodyParams{}
			.linearDamping(6.0f)
			.fixedRotation(true);

		ComponentId backgroundTextureId{};
		ComponentId coffinTextureId{};
		ComponentId fogTextureId{};

		std::vector<Components::Texture*> playerAnimationTextures;
		std::vector<Components::Texture*> enemyAnimationTextures;

		ComponentId musicId{};

		ComponentId sparkingSoundBufferId{};
		ComponentId overchargeSoundBufferId{};
		ComponentId dashSoundBufferId{};
		ComponentId killSoundBufferId{};

		GameParams gameParams{};
		Player playerFrankenstein{};
		Enemy enemyGhost{};
		Enemy enemyChicken{};
		Enemy enemyZombie{};
		SparkingParams sparkingParams{};

		IdGenerator<int> playerIdGenerator{};
		IdGenerator<int> enemyIdGenerator{};

		struct
		{
			bool levelBodiesRendering = false;
			bool presentationTransparency = true;
			bool bodyRendering = false;
		} debug;
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
