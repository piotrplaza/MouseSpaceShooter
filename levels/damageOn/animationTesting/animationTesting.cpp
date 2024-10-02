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

#include <ogl/uniformsUtils.hpp>

#include <glm/gtc/random.hpp>

#include <array>
#include <unordered_map>
#include <fstream>
#include <iostream>

namespace Levels::DamageOn
{
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
			glm::vec2 scale;
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
		} animation;
	};

	struct SparkingParams
	{
		float damageFactor;
		float overheatingRate;
		float coolingRate;
	};

	struct EnemyGhostParams
	{
		int initCount;
		float initHP;
		float initRadius;
		float radiusReductionFactor;
		float minimalRadius;
		int killSpawns;
	};

	namespace
	{
		constexpr const char* paramsPath = "levels/damageOn/animationTesting/params.txt";
		constexpr float mapHSize = 20.0f;

		constexpr float enemyBaseVelocity = 2.0f;
		constexpr float enemyBoostDistance = 15.0f;
		constexpr float enemyBoostFactor = 4.0f;
		constexpr float enemySlowFactor = 0.2f;
		constexpr float enemyDensity = 1.0f;

		constexpr float sparkingDistance = 10.0f;

		constexpr int debrisCount = 10;
		constexpr float debrisDensity = 100.0f;
		constexpr float gamepadDeadZone = 0.1f;
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
			auto& graphicsSettings = Globals::Components().graphicsSettings();
			graphicsSettings.lineWidth = 10.0f;

			auto& musics = Globals::Components().musics();
			musics.emplace("audio/Damage On.ogg", 0.6f).play();

			auto& textures = Globals::Components().staticTextures();
			auto& animatedTextures = Globals::Components().staticAnimatedTextures();

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

			playerAnimatedTextureId = animatedTextures.emplace().getComponentId();

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

			Globals::Components().pauseHandler().handler = [&](bool prevPauseState) {
				auto& audioListener = Globals::Components().audioListener();
				audioListener.setEnabled(!audioListener.isEnabled());

				if (musics.last().isPlaying())
					musics.last().pause();
				else
					musics.last().play();

				return !prevPauseState;
			};

			reload();
		}

		void postSetup()
		{
			auto& camera = Globals::Components().camera2D();
			auto& actors = Globals::Components().actors();
			auto& walls = Globals::Components().staticWalls();
			auto& decorations = Globals::Components().staticDecorations();
			auto& textures = Globals::Components().staticTextures();
			auto& physics = Globals::Components().physics();

			const glm::vec2 levelHSize(textures[backgroundTextureId].loaded.getAspectRatio() * mapHSize, mapHSize);
			camera.targetProjectionHSizeF = camera.details.projectionHSize = camera.details.prevProjectionHSize = 10.0f;
			camera.targetPositionF = [&, levelHSize]() { return glm::clamp(actors[playerId].getOrigin2D(), -levelHSize + camera.details.completeProjectionHSize, levelHSize - camera.details.completeProjectionHSize); };
			camera.positionTransitionFactor = 5.0f;

			const float borderHThickness = 10.0f;
			walls.emplace(Tools::CreateBoxBody({ levelHSize.x + borderHThickness, borderHThickness }, Tools::BodyParams{}.position({ 0.0f, -levelHSize.y - borderHThickness }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			walls.emplace(Tools::CreateBoxBody({ levelHSize.x + borderHThickness, borderHThickness }, Tools::BodyParams{}.position({ 0.0f, levelHSize.y + borderHThickness }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			walls.emplace(Tools::CreateBoxBody({ borderHThickness, levelHSize.y + borderHThickness }, Tools::BodyParams{}.position({ -levelHSize.x - borderHThickness, 0.0f }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			walls.emplace(Tools::CreateBoxBody({ borderHThickness, levelHSize.y + borderHThickness }, Tools::BodyParams{}.position({ levelHSize.x + borderHThickness, 0.0f }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			const float mapScaleFactor = mapHSize / 20.0f;
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
				enemySpawn(glm::linearRand(-levelHSize, levelHSize), enemyGhostParams.initRadius);

			decorations.emplace(Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, levelHSize), CM::StaticTexture(backgroundTextureId), Tools::Shapes2D::CreateTexCoordOfRectangle()).renderLayer = RenderLayer::FarBackground;
		}

		void step()
		{
			auto& keyboard = Globals::Components().keyboard();
			auto& gamepad = Globals::Components().gamepads()[0];
			auto& player = Globals::Components().actors()[playerId];
			auto& physics = Globals::Components().physics();
			auto& animatedTextures = Globals::Components().staticAnimatedTextures();

			for (auto enemyId : enemyIds)
			{
				auto& enemy = Globals::Components().actors()[enemyId];
				const auto direction = player.getOrigin2D() - enemy.getOrigin2D();
				const auto distance = glm::length(direction);
				if (distance > 0.0f)
					enemy.setVelocity(direction / distance * enemyBaseVelocity);
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

			const glm::vec2 newVelocity = direction * playerParams.maxVelocity;
			if (glm::length(newVelocity) > glm::length(player.getVelocity()))
				player.setVelocity(newVelocity);

			auto& playerAnimatedTexture = animatedTextures[playerAnimatedTextureId];
			playerAnimatedTexture.setSpeedScaling(glm::length(direction));
			if (direction.x < 0.0f)
				playerAnimatedTexture.setAdditionalTransformation({}, {}, { -1.0f, 1.0f });
			else if (direction.x > 0.0f)
				playerAnimatedTexture.setAdditionalTransformation({}, {}, { 1.0f, 1.0f });
			else
				playerAnimatedTexture.start(true);

			if (keyboard.pressing[/*VK_CONTROL*/0x11] || gamepad.rTrigger > gamepadTriggerDeadZone || gamepad.lTrigger > gamepadTriggerDeadZone)
			{
				playerFire = true;
				playerAutoFire = false;
			}
			else
				playerFire = false;

			if (keyboard.pressed[/*VK_SHIFT*/0x10] || gamepad.pressed.rShoulder || gamepad.pressed.lShoulder)
				playerAutoFire = !playerAutoFire;

			if (keyboard.pressed[/*VK_SPACE*/0x20] || gamepad.pressed.a)
			{
				auto& dashSound = Globals::Components().sounds()[dashSoundId];
				dashSound.stop().setPlayingOffset(0.35f).setPosition(player.getOrigin2D()).play();
				player.body->ApplyLinearImpulseToCenter(ToVec2<b2Vec2>(direction * playerParams.dash * player.body->GetMass()), true);
			}

			if (keyboard.pressed['T'] || gamepad.pressed.y)
				playerTransparency = !playerTransparency;
			if (keyboard.pressed['B'] || gamepad.pressed.x)
				playerBodyRendering = !playerBodyRendering;
			if (keyboard.pressed['P'] || gamepad.pressed.start)
				reload();

			sparking(playerFire || playerAutoFire);
		}

	private:
		void enemySpawn(glm::vec2 position, float radius)
		{
			auto& actors = Globals::Components().actors();
			auto& physics = Globals::Components().physics();

			const auto enemyId = actors.emplace(Tools::CreateCircleBody(radius, Tools::BodyParams{ defaultBodyParams }
				.bodyType(b2_dynamicBody)
				.density(enemyDensity)
				.position(position)), CM::StaticAnimatedTexture(enemyAnimatedTextureIds[enemyIds.size() % enemyAnimatedTextureIds.size()], {}, {}, glm::vec2(2.6f * radius)))
				.getComponentId();

			enemyIds.insert(enemyId);
			auto& enemy = actors.last();
			enemy.renderingSetupF = createRecursiveFaceRS({ radius * 0.6f, radius });
			auto damageColorFactor = std::make_shared<glm::vec4>(1.0f);
			enemy.colorF = [baseColor = glm::vec4(glm::vec3(glm::linearRand(0.0f, 1.0f)), 1.0f) * 0.8f, damageColorFactor]() { return glm::mix(baseColor, *damageColorFactor, 0.5f); };
			enemy.stepF = [&,
				radius,
				&spark = Globals::Components().dynamicDecorations().emplace(),
				damageColorFactor,
				hp = enemyGhostParams.initHP]() mutable {
				const auto& player = actors[playerId];
				const auto direction = player.getOrigin2D() - enemy.getOrigin2D();
				const auto distance = glm::length(direction);

				enemyBoost(enemy, direction, distance);
				if (enemyTakesDamage(enemy, spark, player, direction, distance, *damageColorFactor, playerFire || playerAutoFire))
				{
					++activeSparks;
					hp -= physics.frameDuration * sparkingParams.damageFactor;
					if (hp <= 0.0f)
					{
						Tools::CreateAndPlaySound(killSoundBufferId, enemy.getOrigin2D(), [&](auto& sound) {
							const float basePitch = 2.0f * enemyGhostParams.initRadius / radius;
							sound.setPitch(glm::linearRand(basePitch, basePitch * 2.0f));
							sound.setVolume(0.7f);
						});
						enemyIds.erase(enemy.getComponentId());
						enemy.state = ComponentState::Outdated;
						spark.state = ComponentState::Outdated;

						const float newRadius = radius * enemyGhostParams.radiusReductionFactor;
						if (newRadius >= enemyGhostParams.minimalRadius)
							for (int i = 0; i < enemyGhostParams.killSpawns; ++i)
								enemySpawn(enemy.getOrigin2D() + glm::circularRand(0.1f), radius * enemyGhostParams.radiusReductionFactor);
					}
				}
			};
		}

		void enemyBoost(auto& enemy, glm::vec2 direction, float distance)
		{
			if (distance > 0.0f && distance < enemyBoostDistance)
				enemy.setVelocity(direction / distance * enemyBaseVelocity * enemyBoostFactor);
		}

		bool enemyTakesDamage(auto& enemy, auto& spark, const auto& player, glm::vec2 direction, float distance, glm::vec4& damageColorFactor, bool fire)
		{
			if (distance <= sparkingDistance && fire && !sparkingOverheated)
			{
				enemy.setVelocity(direction / distance * enemyBaseVelocity * enemySlowFactor);

				spark.vertices = Tools::Shapes2D::CreateVerticesOfLightning(player.getOrigin2D() + glm::diskRand(playerParams.radius * 0.2f),
					enemy.getOrigin2D() + glm::diskRand(enemyGhostParams.minimalRadius), int(20 * distance), 2.0f / glm::sqrt(distance));
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

		void sparking(bool fire)
		{
			const auto& physics = Globals::Components().physics();
			const auto& player = Globals::Components().actors()[playerId];
			auto& sparkingSound = Globals::Components().sounds()[sparkingSoundId];
			auto& overchargeSound = Globals::Components().sounds()[overchargeSoundId];

			if (fire && activeSparks)
			{
				if (!sparkingSound.isPlaying())
					sparkingSound.play();
				sparkingSound.setPosition(player.getOrigin2D());
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
			overchargeSound.setPosition(player.getOrigin2D());
			activeSparks = 0;
		}

		void reload()
		{
			std::ifstream file(paramsPath);
			if (!file.is_open())
				std::cout << "unable to open \"" << paramsPath << "\"" << std::endl;

			std::string key, value;
			while (file >> key >> value)
				params[key] = value;

			std::cout << params.size() << std::endl;
			for (const auto& [key, value] : params)
				std::cout << key << " " << value << std::endl;

			{
				playerParams.startPosition = { std::stof(params["player.startPosition.x"]), std::stof(params["player.startPosition.y"]) };
				playerParams.radius = std::stof(params["player.radius"]);
				playerParams.maxVelocity = std::stof(params["player.maxVelocity"]);
				playerParams.density = std::stof(params["player.density"]);
				playerParams.linearDamping = std::stof(params["player.linearDamping"]);
				playerParams.dash = std::stof(params["player.dash"]);

				playerParams.presentation.radiusProportions = { std::stof(params["player.presentation.radiusProportions.x"]), std::stof(params["player.presentation.radiusProportions.y"]) };
				playerParams.presentation.translation = { std::stof(params["player.presentation.translation.x"]), std::stof(params["player.presentation.translation.y"]) };
				playerParams.presentation.scale = { std::stof(params["player.presentation.scale.x"]), std::stof(params["player.presentation.scale.y"]) };

				playerParams.animation.textureSize = { std::stoi(params["player.animation.textureSize.x"]), std::stoi(params["player.animation.textureSize.y"]) };
				playerParams.animation.framesGrid = { std::stoi(params["player.animation.framesGrid.x"]), std::stoi(params["player.animation.framesGrid.y"]) };
				playerParams.animation.leftTopFrameLeftTopCorner = { std::stoi(params["player.animation.leftTopFrameLeftTopCorner.x"]), std::stoi(params["player.animation.leftTopFrameLeftTopCorner.y"]) };
				playerParams.animation.rightTopFrameLeftEdge = std::stoi(params["player.animation.rightTopFrameLeftEdge"]);
				playerParams.animation.leftBottomFrameTopEdge = std::stoi(params["player.animation.leftBottomFrameTopEdge"]);
				playerParams.animation.frameSize = { std::stoi(params["player.animation.frameSize.x"]), std::stoi(params["player.animation.frameSize.y"]) };
				playerParams.animation.frameDuration = std::stof(params["player.animation.frameDuration"]);
				playerParams.animation.numOfFrames = std::stoi(params["player.animation.numOfFrames"]);
				playerParams.animation.startFrame = std::stoi(params["player.animation.startFrame"]);

				playerParams.animation.direction = params["player.animation.direction"] == "Forward"
					? AnimationData::Direction::Forward
					: AnimationData::Direction::Backward;

				playerParams.animation.mode = params["player.animation.mode"] == "Repeat"
					? AnimationData::Mode::Repeat
					: params["player.animation.mode"] == "Pingpong"
					? AnimationData::Mode::Pingpong
					: AnimationData::Mode::StopOnLastFrame;

				playerParams.animation.textureLayout = params["player.animation.textureLayout"] == "Horizontal"
					? AnimationData::TextureLayout::Horizontal
					: AnimationData::TextureLayout::Vertical;
			}

			{
				sparkingParams.damageFactor = std::stof(params["sparking.damageFactor"]);
				sparkingParams.overheatingRate = std::stof(params["sparking.overheatingRate"]);
				sparkingParams.coolingRate = std::stof(params["sparking.coolingRate"]);
			}

			{
				enemyGhostParams.initCount = std::stoi(params["enemy.ghost.initCount"]);
				enemyGhostParams.initHP = std::stof(params["enemy.ghost.initHP"]);
				enemyGhostParams.initRadius = std::stof(params["enemy.ghost.initRadius"]);
				enemyGhostParams.radiusReductionFactor = std::stof(params["enemy.ghost.radiusReductionFactor"]);
				enemyGhostParams.minimalRadius = std::stof(params["enemy.ghost.minimalRadius"]);
				enemyGhostParams.killSpawns = std::stoi(params["enemy.ghost.killSpawns"]);
			}
			
			auto& actors = Globals::Components().actors();
			auto& physics = Globals::Components().physics();
			const glm::vec2 playerPresentationSize = playerParams.presentation.radiusProportions * playerParams.radius;

			auto reloadPlayer = [&](bool resetPos) {
				auto& player = actors[playerId];
				player.changeBody(Tools::CreateCircleBody(playerParams.radius, Tools::BodyParams{}.linearDamping(playerParams.linearDamping).fixedRotation(true).bodyType(b2_dynamicBody).density(playerParams.density).position(resetPos
					? playerParams.startPosition
					: player.getOrigin2D())));
				player.subsequence.back().vertices = Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, playerPresentationSize.y - playerParams.radius }, playerPresentationSize);
				player.subsequence.back().texture = CM::StaticAnimatedTexture(playerAnimatedTextureId,
					glm::vec2( 0.0f, playerPresentationSize.y - playerParams.radius) + playerPresentationSize * playerParams.presentation.translation, {}, playerPresentationSize * playerParams.presentation.scale);
				player.subsequence.back().modelMatrixF = player.modelMatrixF;
				player.subsequence.back().colorF = [&]() {
					return glm::mix(glm::vec4(1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), sparkingOverheating) * (sparkingOverheated ? (glm::sin(physics.simulationDuration * 20.0f) + 1.0f) / 2.0f : 1.0f);
					};
				player.state = ComponentState::Changed;
			};

			if (!playerId)
			{
				playerId = actors.emplace().getComponentId();
				auto& player = actors.last();
				player.texture = CM::DummyTexture{};
				player.renderF = [this]() { return playerBodyRendering; };
				player.colorF = glm::vec4(0.4f);
				player.subsequence.emplace_back();
				player.subsequence.back().renderingSetupF = [this](auto) { if (!playerTransparency) glDisable(GL_BLEND); return []() mutable { glEnable(GL_BLEND); }; };
				player.posInSubsequence = 1;
				reloadPlayer(true);
			}
			else
				reloadPlayer(false);

			auto& animatedTextures = Globals::Components().staticAnimatedTextures();
			auto& playerAnimatedTexture = animatedTextures[playerAnimatedTextureId];

			playerAnimatedTexture.setAnimationData({ CM::StaticTexture(playerAnimationTextureId), playerParams.animation.textureSize, playerParams.animation.framesGrid, playerParams.animation.leftTopFrameLeftTopCorner,
				playerParams.animation.rightTopFrameLeftEdge, playerParams.animation.leftBottomFrameTopEdge, playerParams.animation.frameSize, playerParams.animation.frameDuration, playerParams.animation.numOfFrames, playerParams.animation.startFrame,
				playerParams.animation.direction, playerParams.animation.mode, playerParams.animation.textureLayout });
			playerAnimatedTexture.start(true);
		}

		const Tools::BodyParams defaultBodyParams = Tools::BodyParams{}
			.linearDamping(6.0f)
			.fixedRotation(true);

		ComponentId playerId{};
		std::unordered_set<ComponentId> enemyIds;

		ComponentId backgroundTextureId{};
		ComponentId coffinTextureId{};
		ComponentId fogTextureId{};

		ComponentId playerAnimationTextureId{};
		ComponentId playerAnimatedTextureId{};

		ComponentId enemyAnimationTextureId{};
		std::array<ComponentId, 10> enemyAnimatedTextureIds;

		ComponentId sparkingSoundId{};
		ComponentId overchargeSoundId{};
		ComponentId dashSoundId{};

		ComponentId killSoundBufferId{};

		bool playerTransparency = true;
		bool playerBodyRendering = false;

		bool playerFire = false;
		bool playerAutoFire = false;

		float sparkingOverheating = 0.0f;
		bool sparkingOverheated = false;
		int activeSparks = 0;

		std::unordered_map<std::string, std::string> params;
		PlayerParams playerParams{};
		SparkingParams sparkingParams{};
		EnemyGhostParams enemyGhostParams{};
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
