#include "nest.hpp"

#include <components/defaults.hpp>
#include <components/graphicsSettings.hpp>
#include <components/systemInfo.hpp>
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
#include <map>
#include <fstream>
#include <iostream>
#include <format>
#include <algorithm>

namespace Levels::DamageOn
{
	struct StartupParams
	{
		bool pixelArt{};
	};

	struct GameParams
	{
		int numOfPlayers{};
		float globalVolume{};
		float musicVolume{};
		float mapHSize{};

		struct Camera
		{
			float minProjectionHSize{};
			float trackingMargin{};
			float positionTransitionFactor{};
			float projectionTransitionFactor{};
		} camera;

		struct Gamepad
		{
			bool firstPlayer{};
			float deadZone{};
			float triggerDeadZone{};
		} gamepad;
	};

	struct Animation
	{
		glm::ivec2 textureSize{};
		glm::ivec2 framesGrid{};
		glm::ivec2 leftTopFrameLeftTopCorner{};
		int rightTopFrameLeftEdge{};
		int leftBottomFrameTopEdge{};
		glm::ivec2 frameSize{};
		float frameDuration{};
		int numOfFrames{};
		int startFrame{};
		AnimationData::Direction direction{};
		AnimationData::Mode mode{};
		AnimationData::TextureLayout textureLayout{};
		glm::vec2 frameTranslation{};
		float frameRotation{};
		glm::vec2 frameScale{};
		int neutralFrame{};
	};

	struct ActorPresentation
	{
		glm::vec2 radiusProportions{};
		glm::vec2 translation{};
		float rotation{};
		glm::vec2 scale{};
		float velocityRotationFactor{};
		float velocityScalingFactor{};
		glm::vec2 weaponOffset{};
	};

	struct Player
	{
		struct Params
		{
			std::string type;

			glm::vec2 startPosition{};
			float radius{};
			float maxVelocity{};
			float density{};
			float linearDamping{};
			float dash{};

			ActorPresentation presentation{};
			Animation animation{};

			struct LoadedBase
			{
				LoadedBase(Components::Texture& animationTexture) :
					animationTexture(animationTexture)
				{
				}

				Components::Texture& animationTexture;
			};

			struct Loaded : LoadedBase
			{
				using LoadedBase::LoadedBase;

				Loaded(Loaded&& loaded) noexcept :
					LoadedBase(std::move(loaded))
				{
					loaded.outdated = true;
				}

				~Loaded()
				{
					if (outdated)
						return;

					animationTexture.state = ComponentState::Outdated;
				}

				bool outdated{};
			};
			std::optional<Loaded> loaded;
		} params;

		struct DataBase
		{
			DataBase(Components::Actor& actor, Components::AnimatedTexture& animatedTexture, Components::Sound& sparkingSound, Components::Sound& overchargeSound, Components::Sound& dashSound) :
				actor(actor),
				animatedTexture(animatedTexture),
				sparkingSound(sparkingSound),
				overchargeSound(overchargeSound),
				dashSound(dashSound)
			{
			}

			Components::Actor& actor;
			Components::AnimatedTexture& animatedTexture;
			Components::Sound& sparkingSound;
			Components::Sound& overchargeSound;
			Components::Sound& dashSound;

			bool fire = false;
			bool autoFire = false;

			float manaOvercharging = 0.0f;
			bool manaOvercharged = false;

			glm::vec4 baseColor{};
			float sideFactor = 1.0f;
			float sideTransition = 1.0f;
			int activeSparks{};
			float angle{};
		};

		struct Data: DataBase
		{
			using DataBase::DataBase;

			Data(Data&& data) noexcept :
				DataBase(std::move(data))
			{
				data.outdated = true;
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

			bool outdated{};
		};

		std::unordered_map<int, Data> idsToData;
	};

	struct EnemyType
	{
		struct Params
		{
			std::string type{};

			int initCount{};
			glm::vec2 startPosition{};
			float initHP{};
			glm::vec2 initRadiusRange{};
			float density{};
			float baseVelocity{};
			float boostDistance{};
			float boostFactor{};
			float slowFactor{};
			float radiusReductionFactor{};
			float minimalRadius{};
			int killSpawns{};

			ActorPresentation presentation{};
			Animation animation{};

			struct LoadedBase
			{
				LoadedBase(Components::Texture& animationTexture) :
					animationTexture(animationTexture)
				{
				}

				Components::Texture& animationTexture;
			};

			struct Loaded : LoadedBase
			{
				using LoadedBase::LoadedBase;

				Loaded(Loaded&& loaded) noexcept :
					LoadedBase(std::move(loaded))
				{
					loaded.outdated = true;
				}

				~Loaded()
				{
					if (outdated)
						return;

					animationTexture.state = ComponentState::Outdated;
				}

				bool outdated{};
			};
			std::optional<Loaded> loaded;
		} params;

		struct DataBase
		{
			DataBase(Components::Actor& actor, Components::AnimatedTexture& animatedTexture) :
				actor(actor),
				animatedTexture(animatedTexture)
			{
			}

			Components::Actor& actor;
			Components::AnimatedTexture& animatedTexture;

			glm::vec4 baseColor{};
			float sideFactor = 1.0f;
			float sideTransition = 1.0f;
			float radius{};
			float angle{};
		};

		struct Data: DataBase
		{
			using DataBase::DataBase;

			Data(Data&& data) noexcept :
				DataBase(std::move(data))
			{
				data.outdated = true;
			}

			~Data()
			{
				if (outdated)
					return;

				actor.state = ComponentState::Outdated;
				animatedTexture.state = ComponentState::Outdated;
			}

			bool outdated{};
		};
		std::unordered_map<int, Data> idsToData;
	};

	struct SparkingParams
	{
		float distance;
		float damageFactor;
		float overchargingRate;
		float coolingRate;
	};

	namespace
	{
		constexpr const char* paramsPath = "levels/damageOn/nest/params.txt";

		constexpr int debrisCount = 5;
		constexpr float debrisDensity = 20.0f;
	}

	class Nest::Impl
	{
	public:
		void setup()
		{
			loadParams();

			auto& defaults = Globals::Components().defaults();
			if (startupParams.pixelArt)
				defaults.forcedResolutionMode = { ResolutionMode::Resolution::H405, ResolutionMode::Scaling::Nearest };

			auto& graphicsSettings = Globals::Components().graphicsSettings();
			graphicsSettings.lineWidth = 6.0f;

			auto& staticTextures = Globals::Components().staticTextures();
			auto& dynamicTextures = Globals::Components().dynamicTextures();
			auto& animatedTextures = Globals::Components().dynamicAnimatedTextures();

			backgroundTextureId = dynamicTextures.emplace("textures/damageOn/nest.jpg", GL_CLAMP_TO_BORDER).getComponentId();
			//dynamicTextures.last().magFilter = GL_NEAREST;
			dynamicTextures.last().scale = glm::vec2(1.0f);
			//dynamicTextures.last().preserveAspectRatio = true;

			coffinTextureId = dynamicTextures.emplace("textures/damageOn/coffin.png", GL_MIRRORED_REPEAT).getComponentId();
			//dynamicTextures.last().magFilter = GL_NEAREST;
			//dynamicTextures.last().scale = glm::vec2(30.0f);
 
			fogTextureId = staticTextures.emplace("textures/damageOn/fog.png", GL_REPEAT).getComponentId();
			staticTextures.last().scale = glm::vec2(0.15f);

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

				if (!prevPauseState)
					musics.last().pause();
				else
					musics.last().play();

				return !prevPauseState;
			};

			reload(false);
		}

		void postSetup()
		{
			const auto& screenInfo = Globals::Components().systemInfo().screen;
			auto& camera = Globals::Components().camera2D();
			auto& dynamicWalls = Globals::Components().dynamicWalls();
			auto& dynamicDecorations = Globals::Components().dynamicDecorations();
			auto& dynamicTextures = Globals::Components().dynamicTextures();
			auto& physics = Globals::Components().physics();

			const glm::vec2 levelHSize(dynamicTextures[backgroundTextureId].loaded.getAspectRatio() * gameParams.mapHSize, gameParams.mapHSize);

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

			alternativeTargetPositionAndProjectionHSizeF = glm::vec3(0.0f, 0.0f, gameParams.mapHSize);

			const float borderHThickness = 10.0f;
			dynamicWalls.emplace(Tools::CreateBoxBody({ levelHSize.x + borderHThickness, borderHThickness }, Tools::BodyParams{}.position({ 0.0f, -levelHSize.y - borderHThickness }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			dynamicWalls.emplace(Tools::CreateBoxBody({ levelHSize.x + borderHThickness, borderHThickness }, Tools::BodyParams{}.position({ 0.0f, levelHSize.y + borderHThickness }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			dynamicWalls.emplace(Tools::CreateBoxBody({ borderHThickness, levelHSize.y + borderHThickness }, Tools::BodyParams{}.position({ -levelHSize.x - borderHThickness, 0.0f }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			dynamicWalls.emplace(Tools::CreateBoxBody({ borderHThickness, levelHSize.y + borderHThickness }, Tools::BodyParams{}.position({ levelHSize.x + borderHThickness, 0.0f }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			const float mapScaleFactor = gameParams.mapHSize / 20.0f;
			const glm::vec2 nestCenter = glm::vec2(1.8f, 2.6f) * mapScaleFactor;
			dynamicWalls.emplace(Tools::CreateCircleBody(6.0f * mapScaleFactor, Tools::BodyParams{}.position(nestCenter))).renderF = [&]() { return debug.hitboxesRendering; };
			dynamicWalls.last().colorF = glm::vec4(0.2f);
			dynamicWalls.last().stepF = [&, &wall = dynamicWalls.last()]() { 
				wall.setEnabled(!bonusBackground); };
			dynamicWalls.emplace(Tools::CreateCircleBody(5.5f * mapScaleFactor, Tools::BodyParams{}.position(nestCenter + glm::vec2(-2.5f) * mapScaleFactor))).renderF = [&]() { return debug.hitboxesRendering; };
			dynamicWalls.last().colorF = glm::vec4(0.2f);
			dynamicWalls.last().stepF = [&, &wall = dynamicWalls.last()]() { wall.setEnabled(!bonusBackground); };
			dynamicWalls.emplace(Tools::CreateCircleBody(3.0f * mapScaleFactor, Tools::BodyParams{}.position(nestCenter + glm::vec2(-6.0f) * mapScaleFactor))).renderF = [&]() { return debug.hitboxesRendering; };
			dynamicWalls.last().colorF = glm::vec4(0.2f);
			dynamicWalls.last().stepF = [&, &wall = dynamicWalls.last()]() { wall.setEnabled(!bonusBackground); };

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
				auto& debris = dynamicWalls.emplace(Tools::CreateBoxBody({ debrisWidth, debrisHeight }, Tools::BodyParams{}.position(glm::linearRand(-levelHSize, levelHSize)).angle(glm::linearRand(0.0f, glm::two_pi<float>()))
					.bodyType(b2_dynamicBody).linearDamping(10.0f).angularDamping(10.0f).density(debrisDensity)), CM::DummyTexture());
				debris.renderF = [&]() { return debug.hitboxesRendering; };
				debris.colorF = glm::vec4(0.2f);
				debris.posInSubsequence = 1;
				auto& debrisPresentation = debris.subsequence.emplace_back();
				debrisPresentation.texture = CM::DynamicTexture(coffinTextureId);
				debrisPresentation.vertices = Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { debrisWidth, debrisHeight });
				debrisPresentation.texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
				debrisPresentation.modelMatrixF = debris.modelMatrixF;
			}

			dynamicDecorations.emplace(Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, levelHSize), CM::DynamicTexture(backgroundTextureId), Tools::Shapes2D::CreateTexCoordOfRectangle()).renderLayer = RenderLayer::FarBackground;
		}

		void step()
		{
			const auto& keyboard = Globals::Components().keyboard();
			const auto& physics = Globals::Components().physics();
			const auto& gamepads = Globals::Components().gamepads();

			const bool anyGamepadBackPressed = [&]() {
				return std::any_of(gamepads.begin(), gamepads.end(), [](const auto& gamepad) { return gamepad.pressed.back; });
			}();

			if (keyboard.pressed['R'] || anyGamepadBackPressed)
				reload();

			std::unordered_map<int, float> minDistances;
			bool anyGamepadPressedX = false;
			bool anyGamepadPressedY = false;
			for (auto& [playerId, playerData] : playerFrankenstein.idsToData)
			{
				const bool keyboardEnabled = gameParams.gamepad.firstPlayer ? true : playerId == 0;
				bool gamepadEnabled = playerId > 3 ? false : gameParams.gamepad.firstPlayer ? true : playerId > 0;
				int gamepadId = gamepadEnabled ? (playerId - !gameParams.gamepad.firstPlayer) : 0;
				auto& gamepad = Globals::Components().gamepads()[gamepadId];

				if (gamepadId >= (int)Globals::Components().gamepads().size())
				{
					gamepadId = 0;
					gamepadEnabled = false;
				}

				auto enemyTypePlayerInteractionsF = [&](auto& enemy) {
					for (auto& [enemyId, enemyData] : enemy.idsToData)
					{
						const auto direction = playerData.actor.getOrigin2D() - enemyData.actor.getOrigin2D();
						const auto distance = glm::length(direction);
						if (auto it = minDistances.find(enemyId); it == minDistances.end() || (distance > 0.0f && distance < it->second))
						{
							enemyData.actor.setVelocity(direction / distance * enemy.params.baseVelocity);
							enemyBoost(enemy, enemyData.actor, direction, distance);
							enemyData.sideFactor = direction.x < 0.0f ? -1.0f : 1.0f;
							minDistances[enemyId] = distance;
						}
					}
				};

				for (auto& enemyType: enemyTypes)
					enemyTypePlayerInteractionsF(enemyType);

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
				else if (vLength < 1.0f)
					playerData.animatedTexture.forceFrame(playerFrankenstein.params.animation.neutralFrame);

				playerData.sideTransition += playerData.sideFactor * physics.frameDuration * 7.0f;
				playerData.sideTransition = glm::clamp(playerData.sideTransition, 0.0f, 1.0f);

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

				if (gamepad.pressed.x * gamepadEnabled)
					anyGamepadPressedX = true;
				if (gamepad.pressed.y * gamepadEnabled)
					anyGamepadPressedY = true;

				sparkingHandler(playerData, playerData.fire || playerData.autoFire);
			}

			if (keyboard.pressed['H'] || anyGamepadPressedX)
				debug.hitboxesRendering = !debug.hitboxesRendering;
			if (keyboard.pressed['T'] || anyGamepadPressedY)
				debug.presentationTransparency = !debug.presentationTransparency;
			if (keyboard.pressed['B'])
			{
				bonusBackground = !bonusBackground;
				auto& backgroundTexture = Globals::Components().dynamicTextures()[backgroundTextureId];
				backgroundTexture.source = bonusBackground
					? "textures/damageOn/sanfranfromairship.jpg"
					: "textures/damageOn/nest.jpg";
				backgroundTexture.state = ComponentState::Changed;
			}
			if (keyboard.pressed['C'])
				std::swap(Globals::Components().camera2D().targetPositionAndProjectionHSizeF, alternativeTargetPositionAndProjectionHSizeF);

			auto enemyTypeActionsF = [&](auto& enemy) {
				for (auto& [enemyId, enemyData] : enemy.idsToData)
				{
					const float vLength = glm::length(enemyData.actor.getVelocity());
					enemyData.angle = -glm::min(glm::quarter_pi<float>(), (vLength * vLength * enemy.params.presentation.velocityRotationFactor));
					enemyData.animatedTexture.setSpeedScaling(enemy.params.presentation.velocityScalingFactor == 0.0f ? 1.0f : glm::length(enemyData.actor.getVelocity() * enemy.params.presentation.velocityScalingFactor));
					enemyData.sideTransition += enemyData.sideFactor * physics.frameDuration * 7.0f;
					enemyData.sideTransition = glm::clamp(enemyData.sideTransition, 0.0f, 1.0f);
				}
			};

			for (auto& enemyType : enemyTypes)
				enemyTypeActionsF(enemyType);
		}

	private:
		void playerSpawn(Player& player, glm::vec2 position)
		{
			const auto playerId = playerIdGenerator.acquire();

			const auto& physics = Globals::Components().physics();
			auto& sounds = Globals::Components().sounds();

			auto& playerActor = Globals::Components().actors().emplace(Tools::CreateCircleBody(player.params.radius, Tools::BodyParams{}
				.linearDamping(player.params.linearDamping).fixedRotation(true).bodyType(b2_dynamicBody).density(player.params.density).position(position)), CM::DummyTexture{});

			auto& playerAnimatedTexture = Globals::Components().dynamicAnimatedTextures().emplace();
			playerAnimatedTexture.setAnimationData({ CM::DynamicTexture(player.params.loaded->animationTexture.getComponentId()), player.params.animation.textureSize, player.params.animation.framesGrid, player.params.animation.leftTopFrameLeftTopCorner,
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

			auto& playerData = player.idsToData.emplace(playerId, Player::Data{ playerActor, playerAnimatedTexture, sparkingSound, overchargingSound, dashSoundBuffer }).first->second;

			playerActor.renderF = [&]() { return debug.hitboxesRendering; };
			playerActor.colorF = glm::vec4(0.4f);
			playerActor.posInSubsequence = 2;

			const auto playerPresentationSize = player.params.presentation.radiusProportions * player.params.radius;
			for (float sideFactor : { -1.0f, 1.0f })
			{
				auto& playerPresentation = playerActor.subsequence.emplace_back();
				playerPresentation.renderingSetupF = [&](auto) {
					bool prevBlend = glProxyIsBlendEnabled();
					if (!debug.presentationTransparency)
						glProxySetBlend(false);
					return [prevBlend]() mutable { glProxySetBlend(prevBlend); };
				};
				playerPresentation.renderF = [&, sideFactor]() { return debug.presentationTransparency || sideFactor == playerData.sideFactor; };
				playerPresentation.vertices = Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, playerPresentationSize);
				playerPresentation.texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
				playerPresentation.texture = CM::DynamicAnimatedTexture(playerData.animatedTexture.getComponentId());
				playerPresentation.modelMatrixF = [&, sideFactor]() {
					playerData.animatedTexture.setAdditionalTransformation(player.params.animation.frameTranslation * glm::vec2(sideFactor, 1.0f),
						player.params.animation.frameRotation * sideFactor, player.params.animation.frameScale * glm::vec2(sideFactor, 1.0f));
					return playerActor.modelMatrixF() * glm::translate(glm::mat4(1.0f), glm::vec3(player.params.presentation.translation, 0.0f) * glm::vec3(playerData.sideFactor, 1.0f, 1.0f))
						* glm::rotate(glm::mat4(1.0f), (player.params.presentation.rotation + playerData.angle) * playerData.sideFactor, glm::vec3(0.0f, 0.0f, 1.0f))
						* glm::scale(glm::mat4(1.0f), glm::vec3(player.params.presentation.scale, 1.0f));
					};

				playerData.baseColor = glm::vec4(1.0f);
				playerPresentation.colorF = [&, sideFactor]() {
					return glm::mix(playerData.baseColor, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), playerData.manaOvercharging) * (playerData.manaOvercharged ? (glm::sin(physics.simulationDuration * 20.0f) + 1.0f) / 2.0f : 1.0f)
						* (sideFactor > 0.0f ? playerData.sideTransition : 1.0f - playerData.sideTransition);
				};
			}
		}

		void enemySpawn(EnemyType& enemy, glm::vec2 position, float radius)
		{
			const auto& physics = Globals::Components().physics();

			auto& enemyAnimatedTexture = Globals::Components().dynamicAnimatedTextures().add({ CM::DynamicTexture(enemy.params.loaded->animationTexture.getComponentId()), enemy.params.animation.textureSize,
				enemy.params.animation.framesGrid, enemy.params.animation.leftTopFrameLeftTopCorner, enemy.params.animation.rightTopFrameLeftEdge, enemy.params.animation.leftBottomFrameTopEdge,
				enemy.params.animation.frameSize, enemy.params.animation.frameDuration, enemy.params.animation.numOfFrames,
				enemy.params.animation.startFrame == -1 ? glm::linearRand(0, enemy.params.animation.numOfFrames - 1) : enemy.params.animation.startFrame,
				enemy.params.animation.direction, enemy.params.animation.mode, enemy.params.animation.textureLayout });
			enemyAnimatedTexture.start(true);

			auto& enemyActor = Globals::Components().actors().emplace(Tools::CreateCircleBody(radius, Tools::BodyParams{ defaultBodyParams }
				.bodyType(b2_dynamicBody)
				.density(enemy.params.density)
				.position(position)), CM::DummyTexture());
			enemyActor.renderF = [&]() { return debug.hitboxesRendering; };
			enemyActor.colorF = glm::vec4(0.4f);
			enemyActor.posInSubsequence = 2;

			const auto enemyId = enemyIdGenerator.acquire();
			auto& enemyData = enemy.idsToData.emplace(enemyId, EnemyType::Data{ enemyActor, enemyAnimatedTexture }).first->second;
			enemyData.radius = radius;

			auto damageColorFactor = std::make_shared<glm::vec4>(1.0f);

			for (float sideFactor : { -1.0f, 1.0f })
			{
				auto& enemyPresentation = enemyActor.subsequence.emplace_back();
				const auto enemyPresentationSize = enemy.params.presentation.radiusProportions * radius;
				enemyPresentation.vertices = Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, enemyPresentationSize);
				enemyPresentation.modelMatrixF = [&, sideFactor]() {
					enemyData.animatedTexture.setAdditionalTransformation(enemy.params.animation.frameTranslation * glm::vec2(sideFactor, 1.0f),
						enemy.params.animation.frameRotation * sideFactor, enemy.params.animation.frameScale * glm::vec2(sideFactor, 1.0f));
					return enemyActor.modelMatrixF() * glm::translate(glm::mat4(1.0f), glm::vec3(enemy.params.presentation.translation, 0.0f) * glm::vec3(sideFactor, 1.0f, 1.0f))
						* glm::rotate(glm::mat4(1.0f), (enemy.params.presentation.rotation + enemyData.angle) * sideFactor, glm::vec3(0.0f, 0.0f, 1.0f))
						* glm::scale(glm::mat4(1.0f), glm::vec3(enemy.params.presentation.scale, 1.0f));
				};

				enemyPresentation.renderF = [&, sideFactor]() { return debug.presentationTransparency || sideFactor == enemyData.sideFactor; };

				if (enemy.params.type == "ghost")
				{
					enemyPresentation.texture = CM::DynamicAnimatedTexture(enemyData.animatedTexture.getComponentId(), {}, {}, glm::vec2(2.6f * radius));
					enemyPresentation.renderingSetupF = [&](auto program) {
						bool prevBlend = glProxyIsBlendEnabled();
						if (!debug.presentationTransparency)
							glProxySetBlend(false);
						return [&, prevBlend, tearDown = createRecursiveFaceRS({ enemyData.radius * 0.6f, enemyData.radius })(program)]() {
							tearDown();
							glProxySetBlend(prevBlend);
						};
					};

					enemyData.baseColor = glm::vec4(glm::vec3(glm::linearRand(0.0f, 1.0f)), 1.0f) * 0.8f;
					enemyPresentation.colorF = [&, damageColorFactor, sideFactor]() {
						return glm::mix(enemyData.baseColor, *damageColorFactor, 0.5f) * (sideFactor > 0.0f ? enemyData.sideTransition : 1.0f - enemyData.sideTransition);
					};
				}
				else
				{
					enemyPresentation.texture = CM::DynamicAnimatedTexture(enemyData.animatedTexture.getComponentId());
					enemyPresentation.texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
					enemyPresentation.renderingSetupF = [&](auto) {
						bool prevBlend = glProxyIsBlendEnabled();
						if (!debug.presentationTransparency)
							glProxySetBlend(false);
						return [prevBlend]() mutable { glProxySetBlend(prevBlend); };
					};

					enemyData.baseColor = glm::vec4(glm::vec3(glm::linearRand(0.5f, 1.0f), glm::linearRand(0.5f, 1.0f), glm::linearRand(0.5f, 1.0f)), 1.0f);
					enemyPresentation.colorF = [&, damageColorFactor, sideFactor]() {
						return glm::mix(enemyData.baseColor, *damageColorFactor, 0.5f) * (sideFactor > 0.0f ? enemyData.sideTransition : 1.0f - enemyData.sideTransition);
					};
				}
			}

			enemyActor.stepF = [&, enemyId, radius, sparks = std::vector<Components::Decoration*>{}, damageColorFactor, hp = enemy.params.initHP]() mutable {
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
						const float basePitch = 0.5f * enemyData.radius / radius;
						sound.setPitch(glm::linearRand(basePitch, basePitch * 20));
						sound.setVolume(0.7f);
						});
					for (auto& spark: sparks)
						spark->state = ComponentState::Outdated;
					enemy.idsToData.erase(enemyId);

					const float newRadius = radius * enemy.params.radiusReductionFactor;
					if (enemy.params.killSpawns > 0 && newRadius >= enemy.params.minimalRadius)
						for (int i = 0; i < enemy.params.killSpawns; ++i)
							enemySpawn(enemy, enemyActor.getOrigin2D() + glm::circularRand(0.1f), radius * enemy.params.radiusReductionFactor);
				}
			};
		}

		void enemyBoost(const EnemyType& enemy, auto& enemyBody, glm::vec2 direction, float distance)
		{
			if (distance > 0.0f && distance < enemy.params.boostDistance)
				enemyBody.setVelocity(direction / distance * enemy.params.baseVelocity * enemy.params.boostFactor);
		}

		bool sparkHandler(const auto& sourceData, const auto& target, auto& targetData, auto& spark, glm::vec2 direction, float distance, glm::vec4& damageColorFactor, bool fire)
		{
			if (distance <= sparkingParams.distance && fire && !sourceData.manaOvercharged)
			{
				const glm::vec2 sourceScalingFactor = playerFrankenstein.params.radius * playerFrankenstein.params.presentation.radiusProportions * glm::vec2(sourceData.sideFactor, 1.0f);
				const glm::vec2 weaponOffset =
					glm::translate(glm::mat4(1.0f), glm::vec3(playerFrankenstein.params.presentation.translation, 0.0f) * glm::vec3(sourceData.sideFactor, 1.0f, 1.0f))
					* glm::rotate(glm::mat4(1.0f), playerFrankenstein.params.presentation.rotation * sourceData.sideFactor, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::scale(glm::mat4(1.0f), glm::vec3(playerFrankenstein.params.presentation.scale * glm::vec2(sourceData.sideFactor, 1.0f), 1.0f))
					* glm::vec4(playerFrankenstein.params.presentation.weaponOffset, 0.0f, 1.0f);

				targetData.actor.setVelocity(direction / distance * target.params.baseVelocity * target.params.slowFactor);
				targetData.animatedTexture.setSpeedScaling(target.params.presentation.velocityScalingFactor == 0.0f ? 1.0f : glm::length(targetData.actor.getVelocity() * target.params.presentation.velocityScalingFactor));

				spark.vertices.clear();
				for (int i = 0; i < 10; ++i)
					Tools::Shapes2D::AppendVerticesOfLightning(spark.vertices, sourceData.actor.getOrigin2D() + weaponOffset + glm::diskRand(glm::min(glm::abs(sourceScalingFactor.x), glm::abs(sourceScalingFactor.y)) * 0.1f),
						targetData.actor.getOrigin2D() + glm::diskRand(targetData.radius * 0.1f), int(20 * distance), 3.0f / glm::sqrt(distance));
				
				spark.bufferDataUsage = GL_DYNAMIC_DRAW;
				spark.drawMode = GL_LINES;
				auto sparkColor = glm::mix(glm::vec4(0.0f, glm::linearRand(0.2f, 0.6f), glm::linearRand(0.4f, 0.8f), 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), sourceData.manaOvercharging);
				spark.colorF = sparkColor * 0.2f;
				damageColorFactor = sparkColor * glm::linearRand(0.0f, 1.0f);
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
				sourceData.manaOvercharging += physics.frameDuration * sparkingParams.overchargingRate;
				if (sourceData.manaOvercharging >= 1.0f)
				{
					sourceData.sparkingSound.pause();
					if (!sourceData.overchargeSound.isPlaying())
						sourceData.overchargeSound.play();
					sourceData.manaOvercharging = 1.0f;
					sourceData.manaOvercharged = true;
				}
			}
			else
			{
				sourceData.sparkingSound.pause();
				sourceData.manaOvercharging -= physics.frameDuration * sparkingParams.coolingRate;
				if (sourceData.manaOvercharging <= 0.0f)
				{
					sourceData.overchargeSound.stop();
					sourceData.manaOvercharging = 0.0f;
					sourceData.manaOvercharged = false;
				}
			}

			sourceData.overchargeSound.setVolume(sourceData.manaOvercharging);
			sourceData.overchargeSound.setPosition(sourceData.actor.getOrigin2D());

			sourceData.activeSparks = 0;
		}

		void loadParams()
		{
			std::ifstream file(paramsPath);
			if (!file.is_open())
				std::cout << "unable to open \"" << paramsPath << "\"" << std::endl;

			std::map<std::string, std::string> keysToValues;
			std::string line;
			while (std::getline(file, line)) {
				std::istringstream iss(line);
				std::string key, value;

				if (iss >> key) {
					std::getline(iss, value);
					value.erase(0, 1);
					keysToValues.emplace(std::move(key), std::move(value));
				}
			}

			std::cout << keysToValues.size() << " params loaded:" << std::endl;
			for (const auto& [key, value] : keysToValues)
				std::cout << key << " " << value << std::endl;
			std::cout << std::endl;

			auto getValue = [&](const std::string& key) {
				try
				{
					return keysToValues.at(key);
				}
				catch (...)
				{
					assert(!"loadParams(): failed to get value for key");
					throw std::runtime_error("loadParams(): Failed to get value for key " + key);
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
				{
					assert(!"loadParams(): unsupported type for key for key");
					throw std::runtime_error("Unsupported type for key " + key);
				}
			};

			loadParam(startupParams.pixelArt, "startup.pixelArt");

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

			auto extractName = [](const std::string& line, const std::string& prefix) -> std::string {
				std::size_t startPos = line.find(prefix);

				if (startPos != std::string::npos) {
					startPos += prefix.length();
					std::size_t endPos = line.find('.', startPos);
					if (endPos != std::string::npos) {
						return line.substr(startPos, endPos - startPos);
					}
				}
				return "";
			};

			auto& dynamicTextures = Globals::Components().dynamicTextures();

			auto loadPlayerParams = [&](auto& playerParams, std::string playerName) {
				loadParam(playerParams.startPosition, std::format("player.{}.startPosition", playerName));
				loadParam(playerParams.radius, std::format("player.{}.radius", playerName));
				loadParam(playerParams.maxVelocity, std::format("player.{}.maxVelocity", playerName));
				loadParam(playerParams.density, std::format("player.{}.density", playerName));
				loadParam(playerParams.linearDamping, std::format("player.{}.linearDamping", playerName));
				loadParam(playerParams.dash, std::format("player.{}.dash", playerName));
				loadPresentationParams(playerParams.presentation, std::format("player.{}", playerName));
				loadAnimationParams(playerParams.animation, std::format("player.{}", playerName));

				if (playerName == "frankenstein")
				{
					auto& texture = dynamicTextures.emplace("textures/damageOn/player 1.png");
					playerParams.loaded.emplace(texture);
					playerParams.loaded->animationTexture.magFilter = GL_LINEAR;
				}
				else
					assert(!"Unsupported player id");
			};

			auto loadEnemyParams = [&]() {
				std::string prevEnemyName;
				for (const auto& [key, value] : keysToValues)
				{
					if (auto enemyName = extractName(key, "enemy."); !enemyName.empty() && enemyName != prevEnemyName)
					{
						auto& enemyTypeParams = enemyTypes.emplace_back().params;
						enemyTypeParams.type = std::move(enemyName);

						if (enemyTypeParams.type == "ghost")
						{
							auto& texture = dynamicTextures.emplace(TextureFile("textures/damageOn/enemy 1.jpg", 4, true, TextureFile::AdditionalConversion::DarkToTransparent, [](float* data, glm::ivec2 size, int numOfChannels) {
								for (int i = 0; i < size.x * size.y; ++i)
								{
									glm::vec4& pixel = reinterpret_cast<glm::vec4*>(data)[i];
									if (pixel.r + pixel.g + pixel.b < 0.2f)
										pixel = {};
								}
								}));
							enemyTypeParams.loaded.emplace(texture);
						}
						else if (enemyTypeParams.type == "chicken")
						{
							auto& texture = dynamicTextures.emplace("textures/damageOn/enemy 2.png");
							enemyTypeParams.loaded.emplace(texture);
						}
						else if (enemyTypeParams.type == "zombie")
						{
							auto& texture = dynamicTextures.emplace("textures/damageOn/enemy 3.png");
							enemyTypeParams.loaded.emplace(texture);
						}
						else if (enemyTypeParams.type == "hive")
						{
							auto& texture = dynamicTextures.emplace("textures/damageOn/enemy 4.png");
							enemyTypeParams.loaded.emplace(texture);
						}
						else
							assert(!"Unsupported enemy id");

						dynamicTextures.last().magFilter = GL_LINEAR;

						loadParam(enemyTypeParams.initCount, std::format("enemy.{}.initCount", enemyTypeParams.type));
						loadParam(enemyTypeParams.startPosition, std::format("enemy.{}.startPosition", enemyTypeParams.type));
						loadParam(enemyTypeParams.initHP, std::format("enemy.{}.initHP", enemyTypeParams.type));
						loadParam(enemyTypeParams.initRadiusRange, std::format("enemy.{}.initRadiusRange", enemyTypeParams.type));
						loadParam(enemyTypeParams.density, std::format("enemy.{}.density", enemyTypeParams.type));
						loadParam(enemyTypeParams.baseVelocity, std::format("enemy.{}.baseVelocity", enemyTypeParams.type));
						loadParam(enemyTypeParams.boostDistance, std::format("enemy.{}.boostDistance", enemyTypeParams.type));
						loadParam(enemyTypeParams.boostFactor, std::format("enemy.{}.boostFactor", enemyTypeParams.type));
						loadParam(enemyTypeParams.slowFactor, std::format("enemy.{}.slowFactor", enemyTypeParams.type));
						loadParam(enemyTypeParams.radiusReductionFactor, std::format("enemy.{}.radiusReductionFactor", enemyTypeParams.type));
						loadParam(enemyTypeParams.minimalRadius, std::format("enemy.{}.minimalRadius", enemyTypeParams.type));
						loadParam(enemyTypeParams.killSpawns, std::format("enemy.{}.killSpawns", enemyTypeParams.type));
						loadPresentationParams(enemyTypeParams.presentation, std::format("enemy.{}", enemyTypeParams.type));
						loadAnimationParams(enemyTypeParams.animation, std::format("enemy.{}", enemyTypeParams.type));

						prevEnemyName = enemyTypeParams.type;
					}
				}
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
			loadEnemyParams();

			loadParam(sparkingParams.distance, "sparking.distance");
			loadParam(sparkingParams.damageFactor, "sparking.damageFactor");
			loadParam(sparkingParams.overchargingRate, "sparking.overchargingRate");
			loadParam(sparkingParams.coolingRate, "sparking.coolingRate");
		}

		void reload(bool loadParams = true)
		{
			auto enemyTypesToPositionsAndRadiuses = [&]() {
				std::unordered_map<std::string, std::vector<glm::vec3>> result;
				for (const auto& enemyType : enemyTypes)
				{
					std::vector<glm::vec3> positionsAndRadiuses;
					positionsAndRadiuses.reserve(enemyType.idsToData.size());
					for (const auto& [enemyId, enemyData] : enemyType.idsToData)
						positionsAndRadiuses.emplace_back(enemyData.actor.getOrigin2D(), enemyData.radius);
					result.emplace(enemyType.params.type, std::move(positionsAndRadiuses));
				}
				return result;
			}();

			if (loadParams)
			{
				enemyTypes.clear();
				this->loadParams();
			}

			auto clearPlayers = [&](auto& playerType) {
				std::vector<glm::vec2> playerPositions;
				if (!playerType.idsToData.empty())
				{
					playerPositions.reserve(playerType.idsToData.size());
					for (const auto& [playerId, playerData] : playerType.idsToData)
						playerPositions.push_back(playerData.actor.getOrigin2D());
					playerType.idsToData.clear();
				}
				return playerPositions;
			};

			auto spawnPlayers = [&](auto& player, const auto& playerPositions) {
				for (int i = 0; i < gameParams.numOfPlayers; ++i)
					playerSpawn(player, i < (int)playerPositions.size() ? playerPositions[i] : player.params.startPosition + glm::circularRand(0.1f));
			};

			auto spawnEnemies = [&](auto& enemyType, std::optional<std::vector<glm::vec3>*> enemyPositionsAndRadiuses = std::nullopt) {
				for (int i = 0; i < enemyType.params.initCount; ++i)
					if (enemyPositionsAndRadiuses && i < (int)(**enemyPositionsAndRadiuses).size())
						enemySpawn(enemyType, glm::vec2((**enemyPositionsAndRadiuses)[i]), (**enemyPositionsAndRadiuses)[i].z);
					else
						enemySpawn(enemyType, enemyType.params.startPosition + glm::circularRand(0.1f), glm::linearRand(enemyType.params.initRadiusRange.x, enemyType.params.initRadiusRange.y));
			};

			playerIdGenerator.reset();
			enemyIdGenerator.reset();

			const auto playerFrankensteinPositions = clearPlayers(playerFrankenstein);
			spawnPlayers(playerFrankenstein, playerFrankensteinPositions);

			for (auto& enemyType : enemyTypes)
			{
				auto it = enemyTypesToPositionsAndRadiuses.find(enemyType.params.type);
				if (it != enemyTypesToPositionsAndRadiuses.end())
					spawnEnemies(enemyType, &it->second);
				else
					spawnEnemies(enemyType);
			}

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

				visibilityReduction(true);
				fullVisibilityDistance(fadingRange.x);
				invisibilityDistance(fadingRange.y);

				return [=]() mutable {
					visibilityReduction(false);
				};
			} };
		}

		const Tools::BodyParams defaultBodyParams = Tools::BodyParams{}
			.linearDamping(6.0f)
			.fixedRotation(true);

		ComponentId backgroundTextureId{};
		ComponentId coffinTextureId{};
		ComponentId fogTextureId{};

		ComponentId musicId{};

		ComponentId sparkingSoundBufferId{};
		ComponentId overchargeSoundBufferId{};
		ComponentId dashSoundBufferId{};
		ComponentId killSoundBufferId{};

		StartupParams startupParams{};
		GameParams gameParams{};
		Player playerFrankenstein{};
		std::vector<EnemyType> enemyTypes{};
		SparkingParams sparkingParams{};

		IdGenerator<int> playerIdGenerator{};
		IdGenerator<int> enemyIdGenerator{};

		struct
		{
			bool presentationTransparency = true;
			bool hitboxesRendering = false;
		} debug;

		bool bonusBackground = false;
		FVec3 alternativeTargetPositionAndProjectionHSizeF;
	};

	Nest::Nest():
		impl(std::make_unique<Impl>())
	{
		impl->setup();
	}

	Nest::~Nest() = default;

	void Nest::postSetup()
	{
		impl->postSetup();
	}

	void Nest::step()
	{
		impl->step();
	}
}
