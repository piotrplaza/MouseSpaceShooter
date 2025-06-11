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
#include <components/appStateHandler.hpp>
#include <components/CollisionHandler.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <globals/components.hpp>

#include <systems/textures.hpp>
#include <globals/systems.hpp>

#include <ogl/shaders/textured.hpp>
#include <globals/shaders.hpp>

#include <tools/Shapes2D.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>
#include <tools/particleSystemHelpers.hpp>
#include <tools/glmHelpers.hpp>

#include <ogl/uniformsUtils.hpp>
#include <ogl/renderingHelpers.hpp>

#include <glm/gtc/random.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <array>
#include <unordered_map>
#include <map>
#include <fstream>
#include <iostream>
#include <format>
#include <algorithm>
#include <numeric>
#include <random>

using namespace std::string_literals;

namespace Levels::DamageOn
{
	namespace
	{
		static const char* paramsPath = "levels/damageOn/nest/params.txt";
		static std::random_device randomDevice;
		static std::mt19937 randomGenerator;

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
					colorUniform.reset(program, "color");
					visibilityReduction.reset(program, "visibilityReduction");
					fullVisibilityDistance.reset(program, "fullVisibilityDistance");
					invisibilityDistance.reset(program, "invisibilityDistance");
				}

				visibilityReduction(true);
				fullVisibilityDistance(fadingRange.x);
				invisibilityDistance(fadingRange.y);

				return [=]() mutable {
					visibilityReduction(false);
				};
			} };
		}

		glm::vec2 getWeaponSourcePoint(const auto& sourceInst)
		{
			return sourceInst.actor.getOrigin2D() + glm::vec2(glm::translate(glm::mat4(1.0f), glm::vec3(sourceInst.type.init.presentation.translation, 0.0f) * glm::vec3(sourceInst.sideFactor, 1.0f, 1.0f))
				* glm::rotate(glm::mat4(1.0f), sourceInst.type.init.presentation.rotation * sourceInst.sideFactor, glm::vec3(0.0f, 0.0f, 1.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(sourceInst.type.init.presentation.scale * glm::vec2(sourceInst.sideFactor, 1.0f), 1.0f))
				* glm::vec4(sourceInst.type.init.weaponOffset, 0.0f, 1.0f));
		}

		bool containsKeyWithPrefix(const auto& orderedMap, const std::string& prefix) {
			auto it = orderedMap.lower_bound(prefix);

			if (it != orderedMap.end() && it->first.rfind(prefix, 0) == 0) {
				return true;
			}
			return false;
		}
	}

	class Nest::Impl
	{
	public:
		void setup()
		{
			loadParams();

			const auto& physics = Globals::Components().physics();
			auto& graphicsSettings = Globals::Components().graphicsSettings();
			graphicsSettings.lineWidth = gameParams.pixelArt ? 1.0f : 10.0f;
			if (gameParams.pixelArt)
				graphicsSettings.forcedResolution = { 640, 360 };

			auto& mainFramebufferRenderer = Globals::Components().mainFramebufferRenderer();
			mainFramebufferRenderer.renderer = Tools::StandardFullscreenRenderer(Globals::Shaders().textured(), Tools::DefaultQuakeIntensity(0.0001f));


			auto& staticTextures = Globals::Components().staticTextures();
			auto& dynamicTextures = Globals::Components().textures();
			auto& staticAnimatedTextures = Globals::Components().staticAnimatedTextures();

			backgroundTextureId = dynamicTextures.emplace("textures/damageOn/" + levelParams.backgroundTexture.substr(1, levelParams.backgroundTexture.length() - 2), GL_CLAMP_TO_BORDER).getComponentId();
			//textures.last().magFilter = GL_NEAREST;
			dynamicTextures.last().scale = glm::vec2(1.0f);
			//textures.last().preserveAspectRatio = true;

			coffinTextureId = staticTextures.emplace("textures/damageOn/coffin.png").getComponentId();
			//textures.last().magFilter = GL_NEAREST;
			//textures.last().scale = glm::vec2(30.0f);

			embryoTextureId = staticTextures.emplace("textures/damageOn/embryo.png").getComponentId();
 
			fogTextureId = staticTextures.emplace("textures/damageOn/fog.png", GL_REPEAT).getComponentId();
			staticTextures.last().scale = glm::vec2(0.15f);

			explosionTextureId = staticTextures.emplace("textures/damageOn/explosion.png").getComponentId();

			fireballTextureId = staticTextures.emplace("textures/damageOn/fireball.png").getComponentId();
			staticTextures.last().minFilter = GL_LINEAR;

			jetfireAnimationTextureId = staticTextures.emplace("textures/damageOn/jetfire.png").getComponentId();
			staticTextures.last().minFilter = GL_LINEAR;

			jetfireAnimatedTextureId = staticAnimatedTextures.add({ CM::Texture(jetfireAnimationTextureId, true), { 992, 1019 }, { 8, 8 }, { 0, 0 }, 897, 895, { 88, 125 }, 0.1f, 64, 0,
					AnimationData::Direction::Forward, AnimationData::Mode::Repeat, AnimationData::TextureLayout::Horizontal }).getComponentId();
			staticAnimatedTextures.last().start(true);

			auto& soundsBuffers = Globals::Components().soundsBuffers();

			sparkingSoundBufferId = soundsBuffers.emplace("audio/Ghosthack Synth - Choatic_C.wav", 2.0f).getComponentId();
			overchargedSoundBufferId = soundsBuffers.emplace("audio/Ghosthack Scrape - Horror_C.wav", 1.5f).getComponentId();
			dashSoundBufferId = soundsBuffers.emplace("audio/Ghosthack Whoosh - 5.wav", 0.4f).getComponentId();
			enemyKillSoundBufferId = soundsBuffers.emplace("audio/Ghosthack Impact - Edge.wav", 0.8f).getComponentId();
			explosionSoundBufferId = soundsBuffers.emplace("audio/Ghosthack Impact - Detonate.wav", 0.75f).getComponentId();
			thrustSoundBufferId = soundsBuffers.emplace("audio/thrust.wav", 1.0f).getComponentId();
			thunderSoundBufferId = soundsBuffers.emplace("audio/Ghosthack Impact - Thunder.wav", 0.5f).getComponentId();

			Tools::CreateFogForeground(5, 0.05f, CM::Texture(fogTextureId, true), glm::vec4(1.0f), [x = 0.0f](int layer) mutable {
				(void)layer;
				const auto& physics = Globals::Components().physics();
				x += physics.frameDuration * 0.01f + layer * 0.00001f;
				const float y = std::sin(x * 30) * 0.01f;
				return glm::vec2(x, y);
			});

			Globals::Components().appStateHandler().pauseF = [](bool prevPauseState) {
				auto& musics = Globals::Components().musics();
				auto& audioListener = Globals::Components().audioListener();
				audioListener.setEnabled(!audioListener.isEnabled());

				if (!prevPauseState)
					musics.last().pause();
				else
					musics.last().play();

				return !prevPauseState;
			};

			Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::shockwaveParticle, Globals::CollisionBits::actor,
				Tools::SkipDuplicatedBodiesCollisions([&](b2Fixture& particleFixture, b2Fixture& actorFixture) {
					auto& shockwaveComponent = Tools::AccessComponent<CM::ShockwaveParticle>(particleFixture);
					auto& actorComponent = Tools::AccessComponent<CM::Actor>(actorFixture);
					const bool playerSource = playerSourceExplosions.contains(shockwaveComponent.getComponentId());

					if (playerSource)
					{
						if (auto it = EnemyType::Inst::actorIdsToEnemyInst.find(actorComponent.getComponentId());
							it != EnemyType::Inst::actorIdsToEnemyInst.end())
							it->second->hp -= shockwavesToDamage[shockwaveComponent.getComponentId()];
					}
					else
					{
						if (auto it = PlayerType::Inst::actorIdsToPlayerInst.find(actorComponent.getComponentId());
							it != PlayerType::Inst::actorIdsToPlayerInst.end())
							it->second->hp -= shockwavesToDamage[shockwaveComponent.getComponentId()];
					}
				}));

			reload(false);
		}

		void postSetup()
		{
			const auto& screenInfo = Globals::Components().systemInfo().screen;
			auto& camera = Globals::Components().camera2D();
			auto& dynamicWalls = Globals::Components().walls();
			auto& dynamicDecorations = Globals::Components().decorations();
			auto& dynamicTextures = Globals::Components().textures();
			auto& physics = Globals::Components().physics();

			const glm::vec2 levelHSize(dynamicTextures[backgroundTextureId].loaded.getAspectRatio() * levelParams.mapHHeight, levelParams.mapHHeight);

			camera.positionTransitionFactor = gameParams.camera.positionTransitionFactor;
			camera.projectionTransitionFactor = gameParams.camera.projectionTransitionFactor;
			camera.targetPositionAndProjectionHSizeF = [&, levelHSize]() {
				glm::vec2 minPos(std::numeric_limits<float>::max());
				glm::vec2 maxPos(std::numeric_limits<float>::lowest());

				for (const auto& [playerId, playerInst]: playerGameComponents.idsToInst)
				{
					const auto pos = playerInst.actor.getOrigin2D();
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

			alternativeTargetPositionAndProjectionHSizeF = glm::vec3(0.0f, 0.0f, levelParams.mapHHeight);

			const float borderHThickness = 10.0f;
			dynamicWalls.emplace(Tools::CreateBoxBody({ levelHSize.x + borderHThickness, borderHThickness }, Tools::BodyParams{}.position({ 0.0f, -levelHSize.y - borderHThickness }).friction(0.0f))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			dynamicWalls.emplace(Tools::CreateBoxBody({ levelHSize.x + borderHThickness, borderHThickness }, Tools::BodyParams{}.position({ 0.0f, levelHSize.y + borderHThickness }).friction(0.0f))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			dynamicWalls.emplace(Tools::CreateBoxBody({ borderHThickness, levelHSize.y + borderHThickness }, Tools::BodyParams{}.position({ -levelHSize.x - borderHThickness, 0.0f }).friction(0.0f))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			dynamicWalls.emplace(Tools::CreateBoxBody({ borderHThickness, levelHSize.y + borderHThickness }, Tools::BodyParams{}.position({ levelHSize.x + borderHThickness, 0.0f }).friction(0.0f))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			for (const auto& boxWall : levelParams.walls.boxes)
			{
				dynamicWalls.emplace(Tools::CreateBoxBody(glm::vec2(boxWall[2], boxWall[3]) * levelParams.mapHHeight, Tools::BodyParams{}.position(glm::vec2(boxWall[0], boxWall[1]) * levelParams.mapHHeight)
					.angle(boxWall[4]))).renderF = [&]() { return debug.hitboxesRendering; };
				dynamicWalls.last().colorF = glm::vec4(0.2f);
				dynamicWalls.last().stepF = [&, &wall = dynamicWalls.last()]() { wall.setEnabled(!bonusLevel); };
			}

			for (const auto& circleWall : levelParams.walls.circles)
			{
				dynamicWalls.emplace(Tools::CreateDiscBody(circleWall.z * levelParams.mapHHeight, Tools::BodyParams{}.position(glm::vec2(circleWall) * levelParams.mapHHeight))).renderF = [&]() { return debug.hitboxesRendering; };
				dynamicWalls.last().colorF = glm::vec4(0.2f);
				dynamicWalls.last().stepF = [&, &wall = dynamicWalls.last()]() { wall.setEnabled(!bonusLevel); };
			}

			for (int i = 0; i < levelParams.debris.count; ++i)
			{
				const float debrisHWidth = glm::linearRand(levelParams.debris.hWidthRange.x, levelParams.debris.hWidthRange.y);
				const glm::vec2 debrisHSize = levelParams.debris.radiusRange == glm::vec2(0.0f)
					? glm::vec2(debrisHWidth, debrisHWidth * glm::linearRand(levelParams.debris.hHeightRatioRange.x, levelParams.debris.hHeightRatioRange.y))
					: glm::vec2(glm::linearRand(levelParams.debris.radiusRange.x, levelParams.debris.radiusRange.y));
				const auto bodyParams = Tools::BodyParams{}.position(glm::linearRand(-levelHSize, levelHSize))
					.angle(glm::linearRand(levelParams.debris.angleRange.x, levelParams.debris.angleRange.y))
					.bodyType(b2_dynamicBody).linearDamping(0.1f).angularDamping(1.0f).density(levelParams.debris.density);
				auto& debris = dynamicWalls.emplace(levelParams.debris.radiusRange == glm::vec2(0.0f)
					? Tools::CreateBoxBody(debrisHSize, bodyParams)
					: Tools::CreateDiscBody(debrisHSize.x, bodyParams)
					, CM::DummyTexture());
				debris.renderF = [&]() { return debug.hitboxesRendering; };
				debris.colorF = glm::vec4(0.2f);
				debris.posInSubsequence = 1;
				auto& debrisPresentation = debris.subsequence.emplace_back();
				debrisPresentation.texture = CM::Texture(embryoTextureId, true);
				debrisPresentation.positions = Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, debrisHSize);
				debrisPresentation.texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
				debrisPresentation.modelMatrixF = debris.modelMatrixF;
				debrisPresentation.colorF = glm::vec4(0.4, 0.4, 0.4, 1.0f);
			}
			
			dynamicDecorations.emplace(Tools::Shapes2D::CreatePositionsOfRectangle(glm::vec2(0.0f), levelHSize), CM::Texture(backgroundTextureId, false), Tools::Shapes2D::CreateTexCoordOfRectangle()).renderLayer = RenderLayer::FarBackground;

			changeLevelMode();
		}

		void step()
		{
			postSteps.clear();
			deferredWeaponsSteps.clear();
			deferredWeaponsPostSteps.clear();

			const auto& keyboard = Globals::Components().keyboard();
			const auto& mouse = Globals::Components().mouse();
			const auto& physics = Globals::Components().physics();
			const auto& gamepads = Globals::Components().gamepads();

			const bool anyGamepadBackPressed = [&]() {
				return std::any_of(gamepads.begin(), gamepads.end(), [](const auto& gamepad) { return gamepad.pressed.back; });
			}();

			if (keyboard.pressed['R'] || anyGamepadBackPressed)
				reload();

			bool anyGamepadPressedX = false;
			bool anyGamepadPressedY = false;

			for (auto& [typeName, type] : weaponGameComponents.typeNamesToTypes)
			{
				if (type.init.archetype == WeaponType::Archetype::Sparking || type.init.archetype == WeaponType::Archetype::Lightning)
					type.cache.decoration.drawMode = GL_LINES;
				else
					type.cache.decoration.drawMode = GL_TRIANGLES;

				type.cache.decoration.bufferDataUsage = GL_DYNAMIC_DRAW;
				type.cache.decoration.positions.clear();
				type.cache.decoration.colors.clear();
				type.cache.decoration.state = ComponentState::Changed;
			}

			for (auto& [playerId, playerInst]: playerGameComponents.idsToInst)
			{
				const bool keyboardApplying = gameParams.gamepad.firstPlayer ? true : playerInst.playerNum == 0;
				const bool gamepadApplying = playerInst.playerNum > 3 ? false : gameParams.gamepad.firstPlayer ? true : playerInst.playerNum > 0;
				const int gamepadId = gamepadApplying ? (playerInst.playerNum - !gameParams.gamepad.firstPlayer) : 0;
				const auto& gamepad = Globals::Components().gamepads()[gamepadId];

				if (gamepad.pressed.x * gamepadApplying)
					anyGamepadPressedX = true;
				if (gamepad.pressed.y * gamepadApplying)
					anyGamepadPressedY = true;

				auto stickDirection = [&](const glm::vec2& stick) {
					const float directionLength = glm::length(stick);
					if (!gamepadApplying || !gamepad.enabled || directionLength <= gameParams.gamepad.deadZone)
						return glm::vec2(0.0f);
					if (directionLength > 1.0f)
						return stick / directionLength;
					const float normalizedLength = (directionLength - gameParams.gamepad.deadZone) / (1.0f - gameParams.gamepad.deadZone);
					return stick / directionLength * normalizedLength;
				};

				const glm::vec2 lStickDirection = stickDirection(gamepad.lStick);
				const glm::vec2 rStickDirection = stickDirection(gamepad.rStick);

				const glm::vec2 keyboardDirection = keyboardApplying
					? [&]() {
						const glm::vec2 direction(
							-(int)(keyboard.pressing[/*VK_LEFT*/0x25] || keyboard.pressing['A']) + (int)(keyboard.pressing[/*VK_RIGHT*/0x27] || keyboard.pressing['D']),
							-(int)(keyboard.pressing[/*VK_DOWN*/0x28] || keyboard.pressing['S']) + (int)(keyboard.pressing[/*VK_UP*/0x26] || keyboard.pressing['W']));
						if (direction == glm::vec2(0.0f))
							return glm::vec2(0.0f);
						return direction / glm::length(direction);
					}()
					: glm::vec2(0.0f);

				const glm::vec2 direction = [&]() {
					const glm::vec2 direction = lStickDirection + keyboardDirection;
					if (glm::length(direction) > 1.0f)
						return direction / glm::length(direction);
					return direction;
				}();

				if (playerInst.actor.isEnabled())
				{
					auto& playerType = playerInst.type;
					const float vLength = glm::length(playerInst.actor.getVelocity());

					playerInst.color = playerInst.baseColor;
					playerInst.angle = -glm::min(glm::quarter_pi<float>(), (vLength * vLength * playerType.init.presentation.velocityRotationFactor));

					playerInst.animatedTexture.setSpeedScaling(playerType.init.presentation.velocityAnimationSpeedFactor == 0.0f ? 1.0f : vLength * playerType.init.presentation.velocityAnimationSpeedFactor);
					if (playerInst.animatedTexture.isForcingFrame())
					{
						playerInst.animatedTexture.forceFrame(std::nullopt);
						playerInst.animatedTexture.start(true);
					}

					if (direction.x < 0.0f)
						playerInst.sideFactor = -1.0f;
					else if (direction.x > 0.0f)
						playerInst.sideFactor = 1.0f;
					else if (direction.y == 0.0f)
						playerInst.animatedTexture.forceFrame(playerType.init.animation.neutralFrame);

					playerInst.sideTransition += playerInst.sideFactor * physics.frameDuration * 7.0f;
					playerInst.sideTransition = glm::clamp(playerInst.sideTransition, 0.0f, 1.0f);

					const glm::vec2 newVelocity = direction * playerType.init.baseVelocity;
					if (glm::length(newVelocity) > glm::length(playerInst.actor.getVelocity()))
						playerInst.actor.setVelocity(newVelocity);

					if (keyboard.pressing[/*VK_CONTROL*/0x11] * keyboardApplying ||
						mouse.pressing.lmb * keyboardApplying ||
						gamepad.rTrigger * gamepadApplying > gameParams.gamepad.triggerDeadZone ||
						gamepad.lTrigger * gamepadApplying > gameParams.gamepad.triggerDeadZone)
					{
						playerInst.fire = true;
						playerInst.autoFire = false;
					}
					else
						playerInst.fire = false;

					if (keyboard.pressed[/*VK_SHIFT*/0x10] * keyboardApplying || gamepad.pressed.rShoulder * gamepadApplying)
						playerInst.autoFire = !playerInst.autoFire;

					if ((keyboard.pressed[/*VK_SPACE*/0x20] * keyboardApplying || (gamepad.pressed.a || gamepad.pressed.lShoulder) * gamepadApplying) && glm::length(direction) > 0.0f && !playerInst.manaOvercharged)
					{
						playerSoundLimitters.dashes->newSound(Tools::CreateAndPlaySound(CM::SoundBuffer(dashSoundBufferId, false), playerInst.actor.getOrigin2D(), [&](auto& sound) {
							sound.setPitch(glm::linearRand(0.8f, 1.2f));
							sound.setPlayingOffset(0.3f);
							sound.setVolume(0.9f);
						}));
						playerInst.actor.body->ApplyLinearImpulseToCenter(ToVec2<b2Vec2>(direction * playerType.init.dash * playerInst.actor.body->GetMass()), true);
					}

					if (mouse.delta != glm::ivec2(0))
					{
						const float mouseSensitive = 0.002f;
						mousePos += mouse.getCartesianDelta() * mouseSensitive;
						const float mouseLength = glm::length(mousePos);
						if (mouseLength > 1.0f)
							mousePos /= mouseLength;
					}

					glm::vec2 aimingDirection = [&]() {
						const float mouseTreshold = 0.2f;
						const float mouseLength = glm::length(mousePos);
						if (!keyboardApplying || mouseLength < mouseTreshold)
							return rStickDirection;
						return glm::normalize(mousePos) * (mouseLength - mouseTreshold) / (1.0f - mouseTreshold) + rStickDirection;
					}();
					if (glm::length(aimingDirection) > 1.0f)
						aimingDirection = glm::normalize(aimingDirection);

					if (aimingDirection != glm::vec2(0.0f))
					{
						playerInst.aiming = true;
						auto& decorations = Globals::Components().decorations();
						const float aimingLength = glm::length(aimingDirection);
						const glm::vec2 nDirection = aimingDirection / aimingLength;
						const float aimingLineDist = 100.0f;
						const float aimingAngle = glm::mix(glm::pi<float>(), playerType.init.minAimingAngle, aimingLength);
						const float hAimingAngle = glm::max(aimingAngle * 0.5f, 0.005f);
						playerInst.aimingD1 = glm::rotate(nDirection, hAimingAngle);
						playerInst.aimingD2 = glm::rotate(nDirection, -hAimingAngle);
						const glm::vec2 orig = getWeaponSourcePoint(playerInst);
						const glm::vec2 d0 = nDirection * aimingLineDist;
						const glm::vec2 d1 = playerInst.aimingD1 * aimingLineDist;
						const glm::vec2 d2 = playerInst.aimingD2 * aimingLineDist;
						std::vector<glm::vec3> aimingArea{ glm::vec3(orig, 0.0f), glm::vec3(orig + d0, 0.0f), glm::vec3(orig + d1, 0.0f), glm::vec3(orig, 0.0f), glm::vec3(orig + d2, 0.0f), glm::vec3(orig + d0, 0.0f) };
						decorations.emplace(std::move(aimingArea));
						const glm::vec4 aimingColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.2f);
						decorations.last().colors = { aimingColor, glm::vec4(0.0f), aimingColor, aimingColor, aimingColor, glm::vec4(0.0f) };
						decorations.last().renderLayer = RenderLayer::Foreground;
						decorations.last().state = ComponentState::LastShot;
					}
					else
						playerInst.aiming = false;
				}

				deferredWeaponsPostSteps.push_back([&]() {
					weaponsPostStep(playerInst);
				});

				std::multimap<float, EnemyType::Inst*> enemiesByDistance;
				for (auto& [enemyId, enemyInst] : enemyGameComponents.idsToInst)
					enemiesByDistance.emplace(glm::distance(getWeaponSourcePoint(playerInst), enemyInst.actor.getOrigin2D()), &enemyInst);

				const auto weaponSourcePoint = getWeaponSourcePoint(playerInst);
				const unsigned seed = randomDevice();
				auto enemySeqsByDistance = std::make_shared<std::multimap<float, int>>();
				auto aimedEnemySeqsByDistance = std::make_shared<std::multimap<float, int>>();
				int enemySeq = 0;
				int aimedEnemySeq = 0;
				for (auto& [distance, enemyInst] : enemiesByDistance)
				{
					enemySeqsByDistance->emplace(distance, enemySeq);
					deferredWeaponsSteps.push_back([&, &enemyInst = *enemyInst, enemySeq, enemySeqsByDistance, aimedEnemySeq, aimedEnemySeqsByDistance, seed]() {
						weaponsStep(playerInst, enemyInst, enemySeq, *enemySeqsByDistance, seed, WeaponType::Aiming::Auto);
						if (aimedEnemySeqsByDistance->empty())
							weaponsStep(playerInst, enemyInst, enemySeq, *enemySeqsByDistance, seed, WeaponType::Aiming::Combined);
					});
					++enemySeq;

					if (playerInst.aiming && (Tools::IsPointBetweenVectors(enemyInst->actor.getOrigin2D(), weaponSourcePoint, playerInst.aimingD1, playerInst.aimingD2) ||
						Tools::DoesRayIntersectCircle(weaponSourcePoint, playerInst.aimingD1, enemyInst->actor.getOrigin2D(), enemyInst->radius) ||
						Tools::DoesRayIntersectCircle(weaponSourcePoint, playerInst.aimingD2, enemyInst->actor.getOrigin2D(), enemyInst->radius)))
					{
						aimedEnemySeqsByDistance->emplace(distance, aimedEnemySeq);
						deferredWeaponsSteps.push_back([&, &enemyInst = *enemyInst, aimedEnemySeq, aimedEnemySeqsByDistance, seed]() {
							weaponsStep(playerInst, enemyInst, aimedEnemySeq, *aimedEnemySeqsByDistance, seed, WeaponType::Aiming::Manual);
							if (!aimedEnemySeqsByDistance->empty())
								weaponsStep(playerInst, enemyInst, aimedEnemySeq, *aimedEnemySeqsByDistance, seed, WeaponType::Aiming::Combined);
						});
						++aimedEnemySeq;
					}
				}
			}

			for (auto& [enemyId, enemyInst] : enemyGameComponents.idsToInst)
			{
				const float vLength = glm::length(enemyInst.actor.getVelocity());
				enemyInst.color = enemyInst.baseColor;
				enemyInst.angle = -glm::min(glm::quarter_pi<float>(), (vLength * vLength * enemyInst.type.init.presentation.velocityRotationFactor));
				enemyInst.animatedTexture.setSpeedScaling(enemyInst.type.init.presentation.velocityAnimationSpeedFactor == 0.0f
					? 1.0f
					: glm::length(enemyInst.actor.getVelocity() * enemyInst.type.init.presentation.velocityAnimationSpeedFactor));
				enemyInst.sideTransition += enemyInst.sideFactor * physics.frameDuration * 7.0f;
				enemyInst.sideTransition = glm::clamp(enemyInst.sideTransition, 0.0f, 1.0f);

				deferredWeaponsPostSteps.push_back([&]() {
					weaponsPostStep(enemyInst);
				});

				std::multimap<float, PlayerType::Inst*> playersByDistance;
				for (auto& [playerId, playerInst]: playerGameComponents.idsToInst)
					playersByDistance.emplace(glm::distance(getWeaponSourcePoint(enemyInst), playerInst.actor.getOrigin2D()), &playerInst);

				bool anyLivePlayer = false;
				for (auto& [distance, playerInst] : playersByDistance)
				{
					if (!playerInst->actor.isEnabled())
						continue;
					const auto direction = playerInst->actor.getOrigin2D() - enemyInst.actor.getOrigin2D();
					if (!enemyBoost(enemyInst.type, enemyInst.actor, direction, distance))
						enemyInst.actor.setVelocity(direction / distance * enemyInst.type.init.baseVelocity);
					enemyInst.sideFactor = direction.x < 0.0f ? -1.0f : 1.0f;
					anyLivePlayer = true;
					break;
				}

				if (!anyLivePlayer)
				{
					const float angle = Tools::StableRandom::Std0Random::HashRange(0, 999, enemyId) / 1000.0f * glm::two_pi<float>();
					const float velocityFactor = Tools::StableRandom::Std0Random::HashRange(500, 1000, enemyId) / 1000.0f;
					enemyInst.actor.setVelocity(glm::vec2(glm::cos(angle), glm::sin(angle)) * velocityFactor * enemyInst.type.init.baseVelocity);
					enemyInst.sideFactor = enemyInst.actor.getVelocity().x < 0.0f ? -1.0f : 1.0f;
					continue;
				}
				
				const unsigned seed = randomDevice();
				auto playerSeqsByDistance = std::make_shared<std::multimap<float, int>>();
				int playerSeq = 0;
				for (auto& [distance, playerInst] : playersByDistance)
				{
					playerSeqsByDistance->emplace(distance, playerSeq);
					deferredWeaponsSteps.push_back([&, &playerInst = *playerInst, playerSeq, playerSeqsByDistance, seed]() {
						weaponsStep(enemyInst, playerInst, playerSeq, *playerSeqsByDistance, seed, WeaponType::Aiming::Auto);
					});
					++playerSeq;
				}
			}

			if (keyboard.pressed['H'] || anyGamepadPressedX)
				debug.hitboxesRendering = !debug.hitboxesRendering;
			if (keyboard.pressed['T'] || anyGamepadPressedY)
				debug.presentationTransparency = !debug.presentationTransparency;
			if (keyboard.pressed['B'])
				changeLevelMode();
			if (keyboard.pressed['C'])
				std::swap(Globals::Components().camera2D().targetPositionAndProjectionHSizeF, alternativeTargetPositionAndProjectionHSizeF);

			for (auto& deferredWeaponsStep : deferredWeaponsSteps)
				deferredWeaponsStep();
			for (auto& deferredWeaponsPostStep : deferredWeaponsPostSteps)
				deferredWeaponsPostStep();
			for (auto& postStep : postSteps)
				postStep();
		}

	private:
		template <typename GameComponentType>
		struct GameComponents
		{
			using Inst = typename GameComponentType::Inst;

			Inst& emplaceInstance(auto&& ... args)
			{
				auto id = idGenerator.acquire();
				auto [it, success] = idsToInst.emplace(id, Inst{ id, args... });
				assert(success);
				return it->second;
			}

			void removeInstance(int id)
			{
				idsToInst.erase(id);
				idGenerator.release(id);
			}

			void resetInstances()
			{
				idsToInst.clear();
				idGenerator.reset();
			}

			void reset()
			{
				typeNamesToTypes.clear();
				resetInstances();
			}

			std::unordered_map<std::string, GameComponentType> typeNamesToTypes;
			std::unordered_map<int, Inst> idsToInst;
			IdGenerator<int> idGenerator{};
		};

		struct GameParams
		{
			bool pixelArt{};
			float globalVolume{};
			float musicVolume{};
			std::string musicFile{};

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

		struct LevelParams
		{
			std::string backgroundTexture{};
			float mapHHeight{};

			struct Walls
			{
				std::vector<std::array<float, 5>> boxes;
				std::vector<glm::vec3> circles;
			} walls;

			struct Debris
			{
				int count{};
				glm::vec2 hWidthRange{};
				glm::vec2 hHeightRatioRange{};
				glm::vec2 radiusRange{};
				glm::vec2 angleRange{};
				float density{};
			} debris;
		};

		struct Animation
		{
			std::string textureFile{};
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

			struct LoadedBase
			{
				LoadedBase(Components::Texture& animationTexture) :
					animationTexture(animationTexture)
				{
				}

				Components::Texture& animationTexture;
				glm::ivec2 textureSize{};
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
		};

		struct ActorPresentation
		{
			glm::vec2 radiusProportions{};
			glm::vec2 translation{};
			float rotation{};
			glm::vec2 scale{};
			float velocityRotationFactor{};
			float velocityAnimationSpeedFactor{};
		};

		struct WeaponType;

		struct PlayerType
		{
			struct Init
			{
				std::string typeName;

				float hp{};
				float radius{};
				float baseVelocity{};
				float density{};
				float linearDamping{};
				float slowFactor{};
				float dash{};
				float coolingRate{};
				float minAimingAngle = 360.0f;
				glm::vec2 weaponOffset{};

				ActorPresentation presentation{};
				Animation animation{};
			} init;

			struct InstBase
			{
				InstBase(int instanceId, int playerNum, PlayerType& type, Components::Actor& actor, Components::AnimatedTexture& animatedTexture, GameComponents<WeaponType>& weaponGameComponents) :
					instanceId(instanceId),
					playerNum(playerNum),
					type(type),
					actor(actor),
					animatedTexture(animatedTexture),
					weaponGameComponents(weaponGameComponents),
					hp(type.init.hp)
				{
				}

				const int instanceId;
				const int playerNum;
				PlayerType& type;
				Components::Actor& actor;
				Components::AnimatedTexture& animatedTexture;
				GameComponents<WeaponType>& weaponGameComponents;

				float hp;
				bool fire = false;
				bool autoFire = false;

				float manaOvercharging = 0.0f;
				bool manaOvercharged = false;

				glm::vec4 baseColor{};
				glm::vec4 color{};
				float sideFactor = 1.0f;
				float sideTransition = 1.0f;
				float angle{};
				bool aiming = false;
				glm::vec2 aimingD1{};
				glm::vec2 aimingD2{};

				std::unordered_set<int> weaponIds;

				Components::Sound* dashSound{};
				Components::Sound* overchargedSound{};
			};

			struct Inst : InstBase
			{
				Inst(int instanceId, int playerNum, PlayerType& type, Components::Actor& actor, Components::AnimatedTexture& animatedTexture, GameComponents<WeaponType>& weaponGameComponents) :
					InstBase(instanceId, playerNum, type, actor, animatedTexture, weaponGameComponents)
				{
					actorIdsToPlayerInst[actor.getComponentId()] = this;
				}

				Inst(Inst&& inst) noexcept :
					InstBase(std::move(inst))
				{
					actorIdsToPlayerInst[actor.getComponentId()] = this;
					inst.outdated = true;
				}

				~Inst()
				{
					if (outdated)
						return;

					actor.state = ComponentState::Outdated;
					animatedTexture.state = ComponentState::Outdated;

					if (dashSound)
						dashSound->immediateFreeResources();
					if (overchargedSound)
						overchargedSound->immediateFreeResources();

					for (const auto weaponId : weaponIds)
						weaponGameComponents.removeInstance(weaponId);

					actorIdsToPlayerInst.erase(actor.getComponentId());
				}

				inline static std::unordered_map<ComponentId, Inst*> actorIdsToPlayerInst;

				bool outdated{};
			};
		};

		struct EnemyType
		{
			struct Init
			{
				std::string typeName;

				float hp{};
				glm::vec2 radiusRange{};
				float density{};
				float baseVelocity{};
				float boostDistance{};
				float boostFactor{};
				float slowFactor{};
				float radiusReductionFactor{};
				float minimalRadius{};
				int killSpawns{};
				float coolingRate{};
				glm::vec2 weaponOffset{};

				ActorPresentation presentation{};
				Animation animation{};
			} init;

			struct InstBase
			{
				InstBase(int instanceId, EnemyType& type, Components::Actor& actor, Components::AnimatedTexture& animatedTexture, GameComponents<WeaponType>& weaponGameComponents) :
					instanceId(instanceId),
					type(type),
					actor(actor),
					animatedTexture(animatedTexture),
					weaponGameComponents(weaponGameComponents),
					hp(type.init.hp)
				{
				}

				const int instanceId;
				EnemyType& type;
				Components::Actor& actor;
				Components::AnimatedTexture& animatedTexture;
				GameComponents<WeaponType>& weaponGameComponents;

				float hp;
				bool fire = false;
				bool autoFire = true;

				float manaOvercharging = 0.0f;
				bool manaOvercharged = false;

				glm::vec4 baseColor{};
				glm::vec4 color{};
				float sideFactor = 1.0f;
				float sideTransition = 1.0f;
				float radius{};
				float angle{};

				std::unordered_set<int> weaponIds;

				Components::Sound* dashSound{};
				Components::Sound* overchargedSound{};
			};

			struct Inst : InstBase
			{
				Inst(int instanceId, EnemyType& type, Components::Actor& actor, Components::AnimatedTexture& animatedTexture, GameComponents<WeaponType>& weaponGameComponents) :
					InstBase(instanceId, type, actor, animatedTexture, weaponGameComponents)
				{
					actorIdsToEnemyInst[actor.getComponentId()] = this;
				}

				Inst(Inst&& inst) noexcept :
					InstBase(std::move(inst))
				{
					actorIdsToEnemyInst[actor.getComponentId()] = this;
					inst.outdated = true;
				}

				~Inst()
				{
					if (outdated)
						return;

					actor.state = ComponentState::Outdated;
					animatedTexture.state = ComponentState::Outdated;

					if (dashSound)
						dashSound->immediateFreeResources();
					if (overchargedSound)
						overchargedSound->immediateFreeResources();

					for (const auto weaponId : weaponIds)
						weaponGameComponents.removeInstance(weaponId);

					actorIdsToEnemyInst.erase(actor.getComponentId());
				}

				inline static std::unordered_map<ComponentId, Inst*> actorIdsToEnemyInst;

				bool outdated{};
			};
		};

		struct WeaponType
		{
			enum class Archetype { Sparking, Lightning, Fireballs };
			enum class Aiming { Auto, Manual, Combined };
			enum class Targeting { Nearest, Random };
			struct Init
			{
				std::string typeName;

				Archetype archetype;
				Aiming aiming;
				Targeting targeting;
				float distance{};
				float damageFactor{};
				float overchargingRate{};
				float reloadTime{};
				float shotDuration{};
				int multishot{};
				float velocity{10};
			} init;

			struct CacheBase
			{
				CacheBase() = default;

				Components::Decoration& decoration = Globals::Components().decorations().emplace();
			};

			struct Cache : CacheBase
			{
				using CacheBase::CacheBase;

				Cache(Cache&& cache) noexcept :
					CacheBase(std::move(cache))
				{
					cache.outdated = true;
				}

				~Cache()
				{
					if (outdated)
						return;

					decoration.state = ComponentState::Outdated;
				}

				bool outdated{};
			} cache;

			struct InstBase
			{
				InstBase(int instanceId, WeaponType& type) :
					instanceId(instanceId),
					type(type),
					multishot(type.init.multishot)
				{
				}

				const int instanceId;
				WeaponType& type;

				int multishot{};
				int shotCounter{};
				float lastShotTime{ -std::numeric_limits<float>::infinity() };
				bool shoting{};

				Components::Sound* fireSound{};
			};

			struct Inst : InstBase
			{
				using InstBase::InstBase;

				Inst(Inst&& inst) noexcept :
					InstBase(std::move(inst))
				{
					inst.outdated = true;
				}

				~Inst()
				{
					if (outdated)
						return;

					if (fireSound)
						fireSound->immediateFreeResources();
				}

				bool outdated{};
			};
		};

		using ActorTypeVariant = std::variant<std::pair<PlayerType*, int>, EnemyType*>;

		struct Actions
		{
			struct Spawner
			{
				ActorTypeVariant actorTypeVariant;
				std::vector<WeaponType*> weaponTypes;
				glm::vec2 position;
				int count;
			};

			std::vector<Spawner> initSpawners;
		};

		void changeLevelMode()
		{
			bonusLevel = !bonusLevel;
			auto& backgroundTexture = Globals::Components().textures()[backgroundTextureId];
			backgroundTexture.source = bonusLevel
				? "textures/damageOn/sanfranfromairship.jpg"s
				: "textures/damageOn/" + levelParams.backgroundTexture.substr(1, levelParams.backgroundTexture.length() - 2);
			backgroundTexture.state = ComponentState::Changed;
		}

		void playerSpawn(PlayerType& playerType, int playerNum, std::vector<WeaponType*>& weaponTypes, glm::vec2 position)
		{
			const auto& physics = Globals::Components().physics();

			auto& playerActor = Globals::Components().actors().emplace(Tools::CreateDiscBody(playerType.init.radius, Tools::BodyParams{}
				.linearDamping(playerType.init.linearDamping).fixedRotation(true).bodyType(b2_dynamicBody).density(playerType.init.density).position(position)), CM::DummyTexture{});
			playerActor.renderF = [&]() { return debug.hitboxesRendering; };
			playerActor.colorF = glm::vec4(0.2f);
			playerActor.posInSubsequence = 2;

			auto& playerAnimatedTexture = Globals::Components().animatedTextures().emplace();
			playerAnimatedTexture.setAnimationData({ CM::Texture(playerType.init.animation.loaded->animationTexture.getComponentId(), false), playerType.init.animation.loaded->textureSize,
				playerType.init.animation.framesGrid, playerType.init.animation.leftTopFrameLeftTopCorner, playerType.init.animation.rightTopFrameLeftEdge, playerType.init.animation.leftBottomFrameTopEdge,
				playerType.init.animation.frameSize, playerType.init.animation.frameDuration, playerType.init.animation.numOfFrames,
				playerType.init.animation.startFrame == -1 ? glm::linearRand(0, playerType.init.animation.numOfFrames - 1) : playerType.init.animation.startFrame,
				playerType.init.animation.direction, playerType.init.animation.mode, playerType.init.animation.textureLayout });
			playerAnimatedTexture.start(true);

			auto& playerInst = playerGameComponents.emplaceInstance(playerNum, playerType, playerActor, playerAnimatedTexture, weaponGameComponents);
			const auto playerPresentationSize = playerType.init.presentation.radiusProportions * playerType.init.radius;

			for (float sideFactor : { -1.0f, 1.0f })
			{
				auto& playerPresentation = playerActor.subsequence.emplace_back();
				playerPresentation.renderingSetupF = [&](auto&) {
					bool prevBlend = glProxyIsBlendEnabled();
					if (!debug.presentationTransparency)
						glProxySetBlend(false);
					return [prevBlend]() mutable { glProxySetBlend(prevBlend); };
				};
				playerPresentation.renderF = [&, sideFactor]() { return debug.presentationTransparency || sideFactor == playerInst.sideFactor; };
				playerPresentation.positions = Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, playerPresentationSize);
				playerPresentation.texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
				playerPresentation.texture = CM::AnimatedTexture(playerInst.animatedTexture.getComponentId(), false);
				playerPresentation.modelMatrixF = [&, sideFactor]() {
					playerInst.animatedTexture.setAdditionalTransformation(playerType.init.animation.frameTranslation * glm::vec2(sideFactor, 1.0f),
						playerType.init.animation.frameRotation * sideFactor, playerType.init.animation.frameScale * glm::vec2(sideFactor, 1.0f));
					return playerActor.modelMatrixF() * glm::translate(glm::mat4(1.0f), glm::vec3(playerType.init.presentation.translation, 0.0f) * glm::vec3(playerInst.sideFactor, 1.0f, 1.0f))
						* glm::rotate(glm::mat4(1.0f), (playerType.init.presentation.rotation + playerInst.angle) * playerInst.sideFactor, glm::vec3(0.0f, 0.0f, 1.0f))
						* glm::scale(glm::mat4(1.0f), glm::vec3(playerType.init.presentation.scale, 1.0f));
					};

				playerInst.color = playerInst.baseColor = glm::vec4(1.0f);
				playerPresentation.colorF = [&, sideFactor]() {
					return glm::mix(playerInst.color, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), playerInst.manaOvercharging) * (playerInst.manaOvercharged ? (glm::sin(physics.simulationDuration * 20.0f) + 1.0f) / 2.0f : 1.0f)
						* (sideFactor > 0.0f ? playerInst.sideTransition : 1.0f - playerInst.sideTransition);
				};
			}

			for (auto weaponType : weaponTypes)
				addWeapon(playerInst, *weaponType);
		}

		void enemySpawn(EnemyType& enemyType, std::vector<WeaponType*>& weaponTypes, glm::vec2 position, float radius)
		{
			const auto& physics = Globals::Components().physics();

			auto& enemyAnimatedTexture = Globals::Components().animatedTextures().add({ CM::Texture(enemyType.init.animation.loaded->animationTexture.getComponentId(), false), enemyType.init.animation.loaded->textureSize,
				enemyType.init.animation.framesGrid, enemyType.init.animation.leftTopFrameLeftTopCorner, enemyType.init.animation.rightTopFrameLeftEdge, enemyType.init.animation.leftBottomFrameTopEdge,
				enemyType.init.animation.frameSize, enemyType.init.animation.frameDuration, enemyType.init.animation.numOfFrames,
				enemyType.init.animation.startFrame == -1 ? glm::linearRand(0, enemyType.init.animation.numOfFrames - 1) : enemyType.init.animation.startFrame,
				enemyType.init.animation.direction, enemyType.init.animation.mode, enemyType.init.animation.textureLayout });
			enemyAnimatedTexture.start(true);

			auto& enemyActor = Globals::Components().actors().emplace(Tools::CreateDiscBody(radius, Tools::BodyParams{ defaultBodyParams }
				.bodyType(b2_dynamicBody)
				.density(enemyType.init.density)
				.position(position)), CM::DummyTexture());
			enemyActor.renderF = [&]() { return debug.hitboxesRendering; };
			enemyActor.colorF = glm::vec4(0.2f);
			enemyActor.posInSubsequence = 2;

			auto& enemyInst = enemyGameComponents.emplaceInstance(enemyType, enemyActor, enemyAnimatedTexture, weaponGameComponents);
			enemyInst.radius = radius;

			for (float sideFactor : { -1.0f, 1.0f })
			{
				auto& enemyPresentation = enemyActor.subsequence.emplace_back();
				const auto enemyPresentationSize = enemyType.init.presentation.radiusProportions * radius;
				enemyPresentation.positions = Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, enemyPresentationSize);
				enemyPresentation.modelMatrixF = [&, sideFactor]() {
					enemyInst.animatedTexture.setAdditionalTransformation(enemyType.init.animation.frameTranslation * glm::vec2(sideFactor, 1.0f),
						enemyType.init.animation.frameRotation * sideFactor, enemyType.init.animation.frameScale * glm::vec2(sideFactor, 1.0f));
					return enemyActor.modelMatrixF() * glm::translate(glm::mat4(1.0f), glm::vec3(enemyType.init.presentation.translation, 0.0f) * glm::vec3(sideFactor, 1.0f, 1.0f))
						* glm::rotate(glm::mat4(1.0f), (enemyType.init.presentation.rotation + enemyInst.angle) * sideFactor, glm::vec3(0.0f, 0.0f, 1.0f))
						* glm::scale(glm::mat4(1.0f), glm::vec3(enemyType.init.presentation.scale, 1.0f));
				};

				enemyPresentation.renderF = [&, sideFactor]() { return debug.presentationTransparency || sideFactor == enemyInst.sideFactor; };

				if (enemyType.init.typeName == "ghost")
				{
					enemyPresentation.texture = CM::AnimatedTexture(enemyInst.animatedTexture.getComponentId(), false, {}, {}, glm::vec2(2.6f * radius));
					enemyPresentation.renderingSetupF = [&](auto program) {
						bool prevBlend = glProxyIsBlendEnabled();
						if (!debug.presentationTransparency)
							glProxySetBlend(false);
						return [&, prevBlend, tearDown = createRecursiveFaceRS({ enemyInst.radius * 0.6f, enemyInst.radius })(program)]() {
							tearDown();
							glProxySetBlend(prevBlend);
						};
					};

					enemyInst.color = enemyInst.baseColor = glm::vec4(glm::vec3(glm::linearRand(0.5f, 1.0f)), 1.0f) * 0.8f;
				}
				else
				{
					enemyPresentation.texture = CM::AnimatedTexture(enemyInst.animatedTexture.getComponentId(), false);
					enemyPresentation.texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
					enemyPresentation.renderingSetupF = [&](auto&) {
						bool prevBlend = glProxyIsBlendEnabled();
						if (!debug.presentationTransparency)
							glProxySetBlend(false);
						return [prevBlend]() mutable { glProxySetBlend(prevBlend); };
					};

					enemyInst.color = enemyInst.baseColor = glm::vec4(glm::vec3(glm::linearRand(0.8f, 1.0f), glm::linearRand(0.8f, 1.0f), glm::linearRand(0.8f, 1.0f)), 1.0f);
				}

				enemyPresentation.colorF = [&, sideFactor]() {
					return glm::mix(enemyInst.color, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), enemyInst.manaOvercharging) * (enemyInst.manaOvercharged ? (glm::sin(physics.simulationDuration * 20.0f) + 1.0f) / 2.0f : 1.0f)
						* (sideFactor > 0.0f ? enemyInst.sideTransition : 1.0f - enemyInst.sideTransition);
				};
			}

			for (auto weaponType : weaponTypes)
				addWeapon(enemyInst, *weaponType);
		}

		bool enemyBoost(const EnemyType& enemyType, auto& enemyActor, glm::vec2 direction, float distance)
		{
			if (distance > 0.0f && distance < enemyType.init.boostDistance)
			{
				enemyActor.setVelocity(direction / distance * enemyType.init.baseVelocity * enemyType.init.boostFactor);
				return true;
			}
			return false;
		}

		void addWeapon(auto& actorInst, WeaponType& weaponType)
		{
			auto& weaponInst = weaponGameComponents.emplaceInstance(weaponType);
			actorInst.weaponIds.insert(weaponInst.instanceId);
		}

		void weaponsStep(auto& sourceInst, auto& targetInst, int targetSeq, const auto& targetSeqsByDistance, unsigned seed, WeaponType::Aiming aiming)
		{
			static constexpr bool playerTarget = std::is_same_v<std::remove_cvref_t<decltype(targetInst)>, PlayerType::Inst>;

			const auto& physics = Globals::Components().physics();
			const auto& sourceActor = sourceInst.actor;
			auto& targetType = targetInst.type;
			auto& targetActor = targetInst.actor; 

			if (sourceActor.isEnabled() && targetActor.isEnabled())
			{
				const auto direction = targetActor.getOrigin2D() - getWeaponSourcePoint(sourceInst);
				const auto distance = glm::length(direction);
				const bool fire = sourceInst.fire || sourceInst.autoFire;

				for (auto weaponId : sourceInst.weaponIds)
				{
					auto& weaponInst = weaponGameComponents.idsToInst.at(weaponId);
					auto& weaponType = weaponInst.type;

					if (weaponType.init.aiming != aiming)
						continue;

					if (fire && distance <= weaponType.init.distance && !sourceInst.manaOvercharged && physics.simulationDuration - weaponInst.lastShotTime >=  weaponInst.type.init.reloadTime)
					{
						randomGenerator.seed(seed);
						shuffledTargetSeqsInRange.resize(std::distance(targetSeqsByDistance.begin(), targetSeqsByDistance.upper_bound(weaponType.init.distance)));
						std::iota(shuffledTargetSeqsInRange.begin(), shuffledTargetSeqsInRange.end(), 0);
						std::shuffle(shuffledTargetSeqsInRange.begin(), shuffledTargetSeqsInRange.end(), randomGenerator);
						shuffledInRangeTargetSeqsMapping.clear();
						{
							auto itVec = shuffledTargetSeqsInRange.begin();
							auto itMap = targetSeqsByDistance.begin();
							for (; itVec != shuffledTargetSeqsInRange.end(); ++itVec, ++itMap)
								shuffledInRangeTargetSeqsMapping[itMap->second] = *itVec;
						}
						const int targetCount = (int)shuffledTargetSeqsInRange.size();

						//for (const auto& [k, v] : shuffledInRangeTargetSeqsMapping)
						//	std::cout << k << ":" << v << " ";
						//std::cout << "; " << targetSeq;

						enum class WeaponState { Skip, Shot, FinalShot } handlerResult = [&]() {
							if (weaponInst.type.init.targeting == WeaponType::Targeting::Nearest)
							{
								if (targetSeq > weaponInst.multishot - 1)
									return WeaponState::Skip;
								if (targetSeq == weaponInst.multishot - 1 || weaponInst.shotCounter == targetCount - 1)
									return WeaponState::FinalShot;
							}
							else if (weaponInst.type.init.targeting == WeaponType::Targeting::Random)
							{
								if (shuffledInRangeTargetSeqsMapping.at(targetSeq) > weaponInst.multishot)
									return WeaponState::Skip;
								if (weaponInst.shotCounter == weaponInst.multishot - 1 || weaponInst.shotCounter == targetCount - 1)
									return WeaponState::FinalShot;
							}
							else
								throw std::runtime_error("Unhandled targeting type: " + (unsigned)weaponInst.type.init.targeting);

							return WeaponState::Shot;
						}();

						if (handlerResult != WeaponState::Skip)
						{
							if (weaponType.init.archetype == WeaponType::Archetype::Sparking)
								sparkingHandler(sourceInst, targetInst, weaponInst, distance);
							else if (weaponType.init.archetype == WeaponType::Archetype::Lightning)
								lightningHandler(sourceInst, targetInst, weaponInst);
							else if (weaponType.init.archetype == WeaponType::Archetype::Fireballs)
								fireballsHandler(sourceInst, targetInst, weaponInst, direction);
							else
								throw std::runtime_error("Unknown weapon archetype: " + (unsigned)weaponType.init.archetype);

							++weaponInst.shotCounter;
						}

						//std::cout << "; " << weaponInst.shotCounter << "; " << (int)handlerResult << std::endl;

						if (handlerResult == WeaponState::FinalShot)
						{
							weaponInst.lastShotTime = physics.simulationDuration;
							//std::cout << std::endl;
						}
					}
				}
			}

			if (targetInst.hp <= 0.0f && targetActor.isEnabled())
			{
				const float basePitch = [&]() {
					if constexpr (playerTarget)
						return 1.0f;
					else
						return ((targetType.init.radiusRange.x + targetType.init.radiusRange.y) / 2.0f) / targetInst.radius;
				}() * 2.0f;

				if constexpr (playerTarget)
				{
					targetActor.setEnabled(false);
					detonate(targetActor, true, 80.0f, 5.0f, Tools::ExplosionParams{}.color({ 0.15f, 0.6f, 0.15f, 0.5f }).presentationScaleFactor(20));
					if (targetInst.overchargedSound)
						targetInst.overchargedSound->state = ComponentState::Outdated;
					if (targetInst.dashSound)
						targetInst.dashSound->state = ComponentState::Outdated;
				}
				else
				{
					detonate(targetActor, false, 10.0f, 0.0f, Tools::ExplosionParams{}.color({ 0.2f, 0.0f, 0.0f, 1.0f }).presentationScaleFactor(2.0f * targetInst.radius).additiveBlending(false).particlesAsSensors(true)
						.renderLayer(RenderLayer::NearMidground), [&]() -> auto& {
						return Tools::CreateAndPlaySound(CM::SoundBuffer(enemyKillSoundBufferId, false), targetActor.getOrigin2D(), [basePitch](auto& sound) {
							sound.setPitch(glm::linearRand(basePitch, basePitch * 2.0f));
							sound.setVolume(0.7f);
							sound.setRemoveOnStop(true);
						});
					}, 50.0f, 10.0f);

					std::vector<WeaponType*> weaponTypes;
					weaponTypes.reserve(targetInst.weaponIds.size());
					for (auto weaponId : targetInst.weaponIds)
						weaponTypes.push_back(&weaponGameComponents.idsToInst.at(weaponId).type);

					targetActor.setEnabled(false);
					postSteps.push_back([&]() { enemyGameComponents.removeInstance(targetInst.instanceId); });

					const float newRadius = targetInst.radius * targetType.init.radiusReductionFactor;
					if (targetType.init.killSpawns > 0 && newRadius >= targetType.init.minimalRadius)
						for (int i = 0; i < targetType.init.killSpawns; ++i)
							enemySpawn(targetType, weaponTypes, targetActor.getOrigin2D() + glm::circularRand(0.1f), newRadius);
				}
			}
		}

		void sparkingHandler(const auto& sourceInst, auto& targetInst, auto& weaponInst, float distance)
		{
			const auto& physics = Globals::Components().physics();
			const auto& sourceType = sourceInst.type;
			const auto& targetType = targetInst.type;
			auto& weaponType = weaponInst.type;

			const auto sourceRadius = [&] {
				if constexpr (requires { sourceInst.radius; })
					return sourceInst.radius;
				else
					return sourceType.init.radius;
			}();

			const auto targetRadius = [&] {
				if constexpr (requires { targetInst.radius; })
					return targetInst.radius;
				else
					return targetType.init.radius;
			}();

			const glm::vec2 sourceScalingFactor = sourceRadius * sourceType.init.presentation.radiusProportions * glm::vec2(sourceInst.sideFactor, 1.0f);

			targetInst.actor.setVelocity(targetInst.actor.getVelocity() * targetType.init.slowFactor);
			targetInst.animatedTexture.setSpeedScaling(targetType.init.presentation.velocityAnimationSpeedFactor == 0.0f ? 1.0f : glm::length(targetInst.actor.getVelocity() * targetType.init.presentation.velocityAnimationSpeedFactor));

			glm::vec4 avgColor{};
			const int iterations = 5;
			for (int i = 0; i < iterations; ++i)
			{
				const glm::vec3 sparkBaseColor = []() {
					if constexpr (std::is_same_v<std::remove_cvref_t<decltype(sourceInst)>, PlayerType::Inst>)
						return glm::vec3(0.0f, glm::linearRand(0.2f, 0.6f), glm::linearRand(0.4f, 0.8f));
					else
						return glm::vec3(0.8f, glm::linearRand(0.1f, 0.3f), glm::linearRand(0.2f, 0.4f));
				}();

				size_t numOfVertices = Tools::Shapes2D::AppendPositionsOfLightning(weaponType.cache.decoration.positions, getWeaponSourcePoint(sourceInst) + glm::diskRand(glm::min(glm::abs(sourceScalingFactor.x), glm::abs(sourceScalingFactor.y)) * 0.1f),
					targetInst.actor.getOrigin2D() + glm::diskRand(targetRadius * 0.1f), int(20 * distance), 4.0f / glm::sqrt(distance));
				const auto sparkColor = glm::mix(glm::vec4(sparkBaseColor, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), sourceInst.manaOvercharging) * glm::linearRand(0.1f, 0.6f);
				weaponType.cache.decoration.colors.insert(weaponType.cache.decoration.colors.end(), numOfVertices, sparkColor);
				avgColor += sparkColor;
			}
			avgColor /= (float)iterations;
			targetInst.color = glm::mix(targetInst.baseColor, avgColor, glm::linearRand(0.5f, 1.0f));
			targetInst.hp -= physics.frameDuration * weaponInst.type.init.damageFactor;
		}

		void lightningHandler(const auto& sourceInst, auto& targetInst, auto& weaponInst)
		{
			static constexpr bool playerTarget = std::is_same_v<std::remove_cvref_t<decltype(targetInst)>, PlayerType::Inst>;

			const auto& physics = Globals::Components().physics();
			const auto& sourceType = sourceInst.type;
			const auto& targetType = targetInst.type;
			auto& weaponType = weaponInst.type;
			auto& lightningDecoration = Globals::Components().decorations().emplace();
			lightningDecoration.drawMode = GL_LINES;

			targetInst.actor.setVelocity(targetInst.actor.getVelocity() * targetType.init.slowFactor);
			targetInst.animatedTexture.setSpeedScaling(targetType.init.presentation.velocityAnimationSpeedFactor == 0.0f ? 1.0f : glm::length(targetInst.actor.getVelocity() * targetType.init.presentation.velocityAnimationSpeedFactor));

			const float distance = 20.0f;
			const glm::vec2 targetPos = targetInst.actor.getOrigin2D();
			const glm::vec2 sourcePos = targetPos + glm::vec2(glm::linearRand(-distance / 2, distance / 2), distance);
			const glm::vec4 targetColor(1.0f);
			const glm::vec4 souceColor(0.0f);

			Tools::Shapes2D::AppendPositionsOfLightning(lightningDecoration.positions, sourcePos, targetPos, int(10 * distance), 5.0f / glm::sqrt(distance));
			
			for (int i = 0; i < (int)lightningDecoration.positions.size(); ++i)
				lightningDecoration.colors.push_back(i != 0 && i % 2 == 0 ? lightningDecoration.colors.back() : glm::mix(souceColor, targetColor, i / (float)(lightningDecoration.positions.size() - 1)));
			const auto lightningColor = glm::mix(glm::vec4(0.0f, 0.0f, glm::linearRand(0.5f, 0.7f), 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), sourceInst.manaOvercharging);
			lightningDecoration.stepF = [&, startTime = physics.simulationDuration, lightningColor]() {
				const float timeElapsed = physics.simulationDuration - startTime;
				const float visibilityTime = 0.2f;
				lightningDecoration.colorF = glm::mix(lightningColor, glm::vec4(0.0f), timeElapsed / visibilityTime);
				if (timeElapsed > visibilityTime)
					lightningDecoration.state = ComponentState::Outdated;
			};

			Tools::CreateSparking(Tools::SparkingParams{}.sourcePoint(targetPos).initVelocity({ 0.0f, 10.0f }).initVelocityRandomMinFactor(0.3f).spreadFactor(0.4f).gravity({ 0.0f, -15.0f }).sparksCount(500).lineWidth(2.0f).trailsScale(3.0f));

			soundLimitters(playerTarget).kills->newSound(Tools::CreateAndPlaySound(CM::SoundBuffer(thunderSoundBufferId, false), targetInst.actor.getOrigin2D(), [](auto& sound) {
				sound.setPitch(glm::linearRand(0.9f, 1.1f));
				sound.setVolume(1.0f);
				sound.setRemoveOnStop(true);
			}));

			targetInst.color = glm::mix(targetInst.baseColor, lightningColor, glm::linearRand(0.5f, 1.0f));
			targetInst.hp -= physics.frameDuration * weaponInst.type.init.damageFactor;
		}

		void fireballsHandler(const auto& sourceInst, auto& targetInst, auto& weaponInst, glm::vec2 deltaPos)
		{
			static constexpr bool playerSource = std::is_same_v<std::remove_cvref_t<decltype(sourceInst)>, PlayerType::Inst>;
			const auto& physics = Globals::Components().physics();

			fireballSpawn(getWeaponSourcePoint(sourceInst), targetInst.actor.getOrigin2D(), glm::normalize(deltaPos), 0.3f, weaponInst, playerSource);
		}

		void fireballSpawn(glm::vec2 startPos, glm::vec2 endPos, glm::vec2 direction, float radius, auto& weaponInst, bool playerSource)
		{
			const auto velocity = direction * weaponInst.type.init.velocity;
			const auto& physics = Globals::Components().physics();
			auto& dynamicActors = Globals::Components().actors();
			auto& fireball = dynamicActors.emplace(Tools::CreateDiscBody(radius,
				Tools::BodyParams{}.position(startPos).bodyType(b2_kinematicBody).sensor(true).bullet(true).velocity(velocity)), CM::DummyTexture());
			fireball.modelMatrixF = [&, scale = 0.0f, nV = glm::normalize(velocity)]() mutable {
				scale += physics.frameDuration * 2.0f;
				scale = std::clamp(scale, 0.0f, 1.0f);
				return glm::rotate(glm::scale(Tools::GetModelMatrix(*fireball.body), glm::vec3(scale)),
					glm::orientedAngle(glm::vec2(0.0f, -1.0f), nV), { 0.0f, 0.0f, 1.0f });
			};
			fireball.renderLayer = RenderLayer::FarForeground;
			fireball.renderF = [&]() { return debug.hitboxesRendering; };
			fireball.colorF = glm::vec4(0.2f);
			auto& thrustSound = Tools::CreateAndPlaySound(CM::SoundBuffer(thrustSoundBufferId, false), [&]() {
				return fireball.getOrigin2D();
			}, [&](auto& sound) {
				sound.setVolume(0.2f);
				sound.setPitch(glm::linearRand(0.8f, 1.0f));
				sound.setLooping(true);
				sound.setPlayingOffset(glm::linearRand(0.0f, 1.0f));
				sound.setRemoveOnStop(true);
			});
			fireball.stepF = [&, startTime = physics.simulationDuration, playerSource, endPos,
				nV = glm::normalize(velocity), multishot = weaponInst.multishot, damageFactor = weaponInst.type.init.damageFactor]() {
				if (glm::distance(fireball.getOrigin2D(), endPos) < 0.5f ||
					glm::dot(nV, glm::normalize(endPos - fireball.getOrigin2D())) <= 0.0f ||
					physics.simulationDuration >= startTime + 5.0f)
				{
					fireball.state = ComponentState::Outdated;
					detonate(fireball, playerSource, damageFactor, damageFactor / 10.0f);
					fireballs.erase(fireball.getComponentId());
				}
			};
			fireball.teardownF = [&, prevTeardownF = std::move(fireball.teardownF)] {
				if (prevTeardownF)
					prevTeardownF();
				thrustSound.state = ComponentState::Outdated;
			};
			fireball.posInSubsequence = 2;
			auto& fireballPresentation = fireball.subsequence.emplace_back();
			fireballPresentation.texture = CM::Texture(fireballTextureId, true, {}, glm::linearRand(0.0f, glm::two_pi<float>()));
			fireballPresentation.positions = Tools::Shapes2D::CreatePositionsOfRectangle(glm::vec2(0.0f), glm::vec2(radius));
			fireballPresentation.texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
			fireballPresentation.modelMatrixF = fireball.modelMatrixF;
			fireballPresentation.colorF = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
			auto& jetfire = fireball.subsequence.emplace_back(Tools::Shapes2D::CreatePositionsOfRectangle(glm::vec2(0.0f), glm::vec2(radius, radius * 4.0f) * 2.0f),
				Tools::Shapes2D::CreateTexCoordOfRectangle(), CM::AnimatedTexture(jetfireAnimatedTextureId, true, {}, 0.0f, { 1.0f, 1.0f }, weaponInst.type.init.velocity));
			jetfire.modelMatrixF = [radius, modelMatrixF = fireball.modelMatrixF]() { return glm::translate(modelMatrixF(), glm::vec3(0.0f, radius * 4.0f, 0.0f) * 2.0f); };
			jetfire.colorF = glm::vec4(1.0f, 0.6f, 0.6f, 1.0f);
			fireballs.insert(fireball.getComponentId());
		}

		void weaponsPostStep(auto& sourceInst)
		{
			static constexpr bool playerSource = std::is_same_v<std::remove_cvref_t<decltype(sourceInst)>, PlayerType::Inst>;

			const auto& physics = Globals::Components().physics();
			auto& sounds = Globals::Components().sounds();
			const auto fire = sourceInst.fire || sourceInst.autoFire;
			bool shoting = false;

			for (auto weaponId : sourceInst.weaponIds)
			{
				auto& weaponInst = weaponGameComponents.idsToInst.at(weaponId);
				auto& weaponType = weaponInst.type;
				auto sourcePosF = [&]() { return sourceInst.actor.getOrigin2D(); };

				auto playFireSound = [&]() {
					if (!weaponInst.fireSound && !weaponInst.shoting)
					{
						weaponInst.fireSound = &soundLimitters(playerSource).weapons->newSound(Tools::CreateAndPlaySound(CM::SoundBuffer(sparkingSoundBufferId, false), sourcePosF, [&](auto& sound) {
							sound.setVolume(0.8f * (1.0f - !playerSource * 0.5f));
							sound.setLooping(true);
							sound.setPlayingOffset(glm::linearRand(0.0f, 1.0f));
							sound.setRemoveOnStop(true);
						}), [&]() {
							weaponInst.fireSound = nullptr;
						});
					}
				};

				if (fire && weaponInst.shotCounter)
				{
					sourceInst.manaOvercharging += physics.frameDuration * weaponInst.type.init.overchargingRate;
					if (sourceInst.manaOvercharging < 1.0f)
					{
						if (weaponType.init.archetype == WeaponType::Archetype::Sparking)
							playFireSound();
						weaponInst.shoting = true;
						shoting = true;
					}
					else
					{
						if (weaponInst.fireSound)
							weaponInst.fireSound->state = ComponentState::Outdated;
						sourceInst.manaOvercharging = 1.0f;
						sourceInst.manaOvercharged = true;
						if (!sourceInst.overchargedSound)
							sourceInst.overchargedSound = &soundLimitters(playerSource).overcharges->newSound(Tools::CreateAndPlaySound(CM::SoundBuffer(overchargedSoundBufferId, false), sourcePosF, [&](auto& sound) {
								sound.setVolume(sourceInst.manaOvercharging * (1.0f - !playerSource * 0.5f));
								sound.setPitch(glm::linearRand(0.9f, 1.1f));
								sound.setLooping(true);
								sound.setPlayingOffset(glm::linearRand(0.0f, 1.0f));
								sound.setRemoveOnStop(true);
							}), [&]() {
								sourceInst.overchargedSound = nullptr;
							});
						weaponInst.shoting = false;
					}
				}
				else
				{
					if (weaponInst.fireSound)
						weaponInst.fireSound->state = ComponentState::Outdated;
					weaponInst.shoting = false;
				}

				if (sourceInst.overchargedSound)
				{
					if (sourceInst.manaOvercharged)
					{
						sourceInst.overchargedSound->setVolume(sourceInst.manaOvercharging * (1.0f - !playerSource * 0.5f));
						sourceInst.overchargedSound->setPosition(sourcePosF());
					}
					else
						sourceInst.overchargedSound->state = ComponentState::Outdated;
				}

				weaponInst.shotCounter = 0;
			}

			if (!shoting)
			{
				sourceInst.manaOvercharging -= physics.frameDuration * sourceInst.type.init.coolingRate;
				if (sourceInst.manaOvercharging <= 0.0f)
				{
					sourceInst.manaOvercharging = 0.0f;
					sourceInst.manaOvercharged = false;
				}
			}
		}

		void detonate(auto& actor, bool playerSource, float power, float damage, Tools::ExplosionParams explosionParams = Tools::ExplosionParams{}.color(glm::vec4(0.5f)).presentationScaleFactor(20.0f),
			std::function<Components::Sound& ()> customCreateAndPlaySoundF = nullptr, float initExplosionVelocityFactor = 200.0f, float explosionDurationFactor = 2.0f)
		{
			if (playerSource)
				explosionParams.beginCallback([&, damage](auto& shockwave) {
					playerSourceExplosions.insert(shockwave.getComponentId());
					shockwavesToDamage[shockwave.getComponentId()] = damage;
				}).endCallback([&](auto& shockwave) {
					playerSourceExplosions.erase(shockwave.getComponentId());
					shockwavesToDamage.erase(shockwave.getComponentId());
				});
			else
				explosionParams.beginCallback([&, damage](auto& shockwave) {
					enemySourceExplosions.insert(shockwave.getComponentId());
					shockwavesToDamage[shockwave.getComponentId()] = damage;
				}).endCallback([&](auto& shockwave) {
					enemySourceExplosions.erase(shockwave.getComponentId());
					shockwavesToDamage.erase(shockwave.getComponentId());
				});

			const float explosionPower = power / 100.0f;

			explosionParams.center(actor.getMassCenter()).explosionTexture(CM::Texture(explosionTextureId, true))
				.numOfParticles(int(std::max(32 * explosionPower, 3.0f))).particlesRadius(1.0f).particlesDensity(10.0f * explosionPower).particlesAsBullets(false).deferredExectution(false)
				.initExplosionVelocity(initExplosionVelocityFactor * explosionPower).explosionDuration(std::max(explosionDurationFactor * explosionPower, 0.75f)).particlesPerDecoration(1);

			Tools::CreateExplosion(explosionParams);
			soundLimitters(playerSource).kills->newSound(customCreateAndPlaySoundF
				? customCreateAndPlaySoundF()
				: Tools::CreateAndPlaySound(CM::SoundBuffer(explosionSoundBufferId, false), actor.getOrigin2D(), [explosionPower](auto& sound) {
					sound.setPitch(glm::linearRand(1.0f, 2.0f) / sqrt(explosionPower));
					sound.setVolume(sqrt(explosionPower));
					sound.setRemoveOnStop(true);
				}));
		}

		void loadParams()
		{
			std::ifstream file(paramsPath);
			if (!file.is_open())
				std::cout << "unable to open \"" << paramsPath << "\"" << std::endl;

			std::multimap<std::string, std::string> keysToValues;
			std::string line;
			while (std::getline(file, line))
			{
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
				auto range = keysToValues.equal_range(key);
				if (range.first == range.second)
					throw std::runtime_error("loadParams(): Key " + key + " not found");
				if (std::distance(range.first, range.second) > 1)
					throw std::runtime_error("loadParams(): Multiple values for key " + key);
				return range.first->second;
			};

			auto getValues = [&](const std::string& key) {
				auto range = keysToValues.equal_range(key);
				if (range.first == range.second)
					throw std::runtime_error("loadParams(): Key " + key + " not found");
				std::vector<std::string> values;
				values.reserve(std::distance(range.first, range.second));
				for (auto it = range.first; it != range.second; ++it)
					values.push_back(it->second);
				return values;
			};

			auto loadParam = [&](auto& param, const std::string& key, bool mandatory = true) {
				using ParamType = std::remove_reference_t<decltype(param)>;

				try
				{
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
					else if constexpr (std::is_same_v<ParamType, std::array<std::string, 4>>)
					{
						const auto value = getValue(key);
						const auto spacePos1 = value.find(' ');
						const auto spacePos2 = value.find(' ', spacePos1 + 1);
						const auto spacePos3 = value.find(' ', spacePos2 + 1);
						param = { value.substr(0, spacePos1), value.substr(spacePos1 + 1, spacePos2 - spacePos1 - 1),
							value.substr(spacePos2 + 1, spacePos3 - spacePos2 - 1), value.substr(spacePos3 + 1) };
					}
					else if constexpr (std::is_same_v<ParamType, std::vector<std::array<float, 5>>>)
					{
						const auto values = getValues(key);
						param.reserve(values.size());
						for (const auto& value : values)
						{
							const auto spacePos1 = value.find(' ');
							const auto spacePos2 = value.find(' ', spacePos1 + 1);
							const auto spacePos3 = value.find(' ', spacePos2 + 1);
							const auto spacePos4 = value.find(' ', spacePos3 + 1);
							param.push_back({
								Tools::Stof(value.substr(0, spacePos1)),
								Tools::Stof(value.substr(spacePos1 + 1, spacePos2 - spacePos1 - 1)),
								Tools::Stof(value.substr(spacePos2 + 1, spacePos3 - spacePos2 - 1)),
								Tools::Stof(value.substr(spacePos3 + 1, spacePos4 - spacePos3 - 1)),
								Tools::Stof(value.substr(spacePos4 + 1)) });
						}
					}
					else if constexpr (std::is_same_v<ParamType, std::vector<glm::vec3>>)
					{
						const auto values = getValues(key);
						param.reserve(values.size());
						for (const auto& value : values)
						{
							const auto spacePos1 = value.find(' ');
							const auto spacePos2 = value.find(' ', spacePos1 + 1);
							param.push_back({
								Tools::Stof(value.substr(0, spacePos1)),
								Tools::Stof(value.substr(spacePos1 + 1, spacePos2 - spacePos1 - 1)),
								Tools::Stof(value.substr(spacePos2 + 1)) });
						}
					}
					else if constexpr (std::is_same_v<ParamType, std::vector<Actions::Spawner>>)
					{
						const auto values = getValues(key);
						param.reserve(values.size());
						for (const auto& value : values)
						{
							std::deque<std::string> params = [&]() {
								std::deque<std::string> result;
								std::istringstream iss(value);
								std::string param;
								while (getline(iss, param, ' '))
									result.push_back(std::move(param));
								return result;
							}();

							const auto dotPos = params.front().find('.');
							const auto generalType = params.front().substr(0, dotPos);
							const auto specificType = params.front().substr(dotPos + 1);

							params.pop_front();

							const auto playerNum = [&]() {
								if (generalType != "player")
									return -1;

								const auto result = Tools::Stoi(params.front());
								if (result < 0 || result > 3)
									throw std::runtime_error("loadParams(): Player number " + std::to_string(result) + " out of range");
								params.pop_front();
								return result;
							}();

							auto actorTypeVariant = [&]() {
								try {
									if (generalType == "player")
										return ActorTypeVariant(std::make_pair(&playerGameComponents.typeNamesToTypes.at(specificType), playerNum));
									if (generalType == "enemy")
										return ActorTypeVariant(&enemyGameComponents.typeNamesToTypes.at(specificType));

									throw std::runtime_error("loadParams(): Actor general type " + generalType + " unsupported");
								}
								catch (...) { throw std::runtime_error("loadParams(): Actor type " + generalType + "." + specificType + " not found"); }
							}();

							const auto pos = [&]() {
								const auto x = Tools::Stof(params.front());
								params.pop_front();
								const auto y = Tools::Stof(params.front());
								params.pop_front();
								return glm::vec2(x, y);
							}();

							const auto count = Tools::Stoi(params.front());
							params.pop_front();

							std::vector<WeaponType*> weaponTypes;
							while (!params.empty())
							{
								if (params.front().substr(0, 7) != "weapon.")
								{
									params.pop_front();
									continue;
								}

								const auto dotPos = params.front().find('.');
								const auto weaponType = params.front().substr(dotPos + 1);
								try { weaponTypes.push_back(&weaponGameComponents.typeNamesToTypes.at(weaponType)); }
								catch (...) { throw std::runtime_error("loadParams(): Weapon type " + weaponType + " not found"); }
								params.pop_front();
							}

							param.emplace_back(actorTypeVariant, weaponTypes, pos, count);
						}
					}
					else if constexpr (std::is_same_v<ParamType, AnimationData::Direction>)
					{
						const std::string value = getValue(key);
						param = value == "forward"
							? AnimationData::Direction::Forward
							: param = value == "backward"
							? AnimationData::Direction::Backward
							: throw std::runtime_error("loadParams(): Unsupported value " + value + " for key " + key);
					}
					else if constexpr (std::is_same_v<ParamType, AnimationData::Mode>)
					{
						const std::string value = getValue(key);
						param = value == "repeat"
							? AnimationData::Mode::Repeat
							: value == "pingpong"
							? AnimationData::Mode::Pingpong
							: value == "stopOnLastFrame"
							? AnimationData::Mode::StopOnLastFrame
							: throw std::runtime_error("loadParams(): Unsupported value " + value + " for key " + key);
					}
					else if constexpr (std::is_same_v<ParamType, AnimationData::TextureLayout>)
					{
						const std::string value = getValue(key);
						param = value == "horizontal"
							? AnimationData::TextureLayout::Horizontal
							: value == "vertical"
							? AnimationData::TextureLayout::Vertical
							: throw std::runtime_error("loadParams(): Unsupported value " + value + " for key " + key);
					}
					else if constexpr (std::is_same_v<ParamType, WeaponType::Archetype>)
					{
						const std::string value = getValue(key);
						param = value == "sparking"
							? WeaponType::Archetype::Sparking
							: value == "lightning"
							? WeaponType::Archetype::Lightning
							: value == "fireballs"
							? WeaponType::Archetype::Fireballs
							: throw std::runtime_error("loadParams(): Unsupported value " + value + " for key " + key);
					}
					else if constexpr (std::is_same_v<ParamType, WeaponType::Aiming>)
					{
						const std::string value = getValue(key);
						param = value == "auto"
							? WeaponType::Aiming::Auto
							: value == "manual"
							? WeaponType::Aiming::Manual
							: value == "combined"
							? WeaponType::Aiming::Combined
							: throw std::runtime_error("loadParams(): Unsupported value " + value + " for key " + key);
					}
					else if constexpr (std::is_same_v<ParamType, WeaponType::Targeting>)
					{
						const std::string value = getValue(key);
						param = value == "nearest"
							? WeaponType::Targeting::Nearest
							: value == "random"
							? WeaponType::Targeting::Random
							: throw std::runtime_error("loadParams(): Unsupported value " + value + " for key " + key);
					}
					else
						throw std::runtime_error("Unsupported value for key " + key);
				}
				catch (const std::runtime_error& e)
				{
					if (mandatory || e.what() != "loadParams(): Key " + key + " not found")
						throw e;
				}
				catch (...)
				{
					throw;
				}
			};

			auto& dynamicTextures = Globals::Components().textures();

			auto loadPresentationParams = [&](auto& presentationParams, std::string actorName) {
				loadParam(presentationParams.radiusProportions, std::format("{}.presentation.radiusProportions", actorName));
				loadParam(presentationParams.translation, std::format("{}.presentation.translation", actorName));
				loadParam(presentationParams.rotation, std::format("{}.presentation.rotation", actorName));
				loadParam(presentationParams.scale, std::format("{}.presentation.scale", actorName));
				loadParam(presentationParams.velocityRotationFactor, std::format("{}.presentation.velocityRotationFactor", actorName));
				presentationParams.velocityRotationFactor = glm::radians(presentationParams.velocityRotationFactor);
				loadParam(presentationParams.velocityAnimationSpeedFactor, std::format("{}.presentation.velocityAnimationSpeedFactor", actorName));
			};

			auto loadAnimationParams = [&](auto& animationParams, std::string actorName) {
				const bool containsAnimation = containsKeyWithPrefix(keysToValues, actorName + ".animation.");
				const bool containsImage = containsKeyWithPrefix(keysToValues, actorName + ".image.");
				if (containsAnimation && !containsImage)
				{
					loadParam(animationParams.textureFile, std::format("{}.animation.textureFile", actorName));
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
				}
				else if (!containsAnimation && containsImage)
				{
					loadParam(animationParams.textureFile, std::format("{}.image.textureFile", actorName));
					animationParams.framesGrid = { 1, 1 };
					animationParams.leftTopFrameLeftTopCorner = { 0, 0 };
					animationParams.rightTopFrameLeftEdge = 0;
					animationParams.leftBottomFrameTopEdge = 0;
					animationParams.frameSize = { 0, 0 };
					animationParams.frameDuration = 0.0f;
					animationParams.numOfFrames = 1;
					animationParams.startFrame = 0;
					animationParams.direction = AnimationData::Direction::Forward;
					animationParams.mode = AnimationData::Mode::StopOnLastFrame;
					animationParams.textureLayout = AnimationData::TextureLayout::Horizontal;
					loadParam(animationParams.frameTranslation, std::format("{}.image.frameTranslation", actorName));
					loadParam(animationParams.frameRotation, std::format("{}.image.frameRotation", actorName));
					loadParam(animationParams.frameScale, std::format("{}.image.frameScale", actorName));
					animationParams.neutralFrame = 0;
				}
				else
					throw std::runtime_error("loadParams(): Actor " + actorName + " must have either animation or image parameters");

				const auto [texturePath, additionalConversion] = [](const std::string& text) -> std::pair<std::string, std::string> {
					size_t start = text.find('"');
					if (start == std::string::npos)
						return {};
					size_t end = text.find('"', start + 1);
					if (end == std::string::npos)
						return {};
					const std::string remains = text.substr(end + 1);
					return { "textures/damageOn/" + text.substr(start + 1, end - start - 1), remains.empty() ? "" : remains.substr(1) };
				}(animationParams.textureFile);

				if (additionalConversion == "darkToTransparent")
					dynamicTextures.emplace(TextureFile(texturePath, 4, true, TextureFile::AdditionalConversion::DarkToTransparent, [](float* data, glm::ivec2 size, int numOfChannels) {
						for (int i = 0; i < size.x * size.y; ++i)
						{
							glm::vec4& pixel = reinterpret_cast<glm::vec4*>(data)[i];
							if (pixel.r + pixel.g + pixel.b < 0.2f)
								pixel = {};
						}
					}));
				else
					dynamicTextures.emplace(texturePath);

				dynamicTextures.last().magFilter = GL_LINEAR;
				animationParams.loaded.emplace(dynamicTextures.last());
				animationParams.loaded->textureSize = Globals::Systems().textures().loadFile(TextureFile(texturePath)).size;
				if (animationParams.frameSize == glm::ivec2(0))
					animationParams.frameSize = animationParams.loaded->textureSize;
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

			auto loadPlayerParams = [&]() {
				std::string prevPlayerName;
				for (const auto& [key, value] : keysToValues)
				{
					if (auto playerName = extractName(key, "player."); !playerName.empty() && playerName != prevPlayerName)
					{
						auto& typeInit = playerGameComponents.typeNamesToTypes.emplace(playerName, PlayerType{}).first->second.init;
						typeInit.typeName = std::move(playerName);

						loadParam(typeInit.hp, std::format("player.{}.hp", typeInit.typeName));
						loadParam(typeInit.radius, std::format("player.{}.radius", typeInit.typeName));
						loadParam(typeInit.baseVelocity, std::format("player.{}.baseVelocity", typeInit.typeName));
						loadParam(typeInit.slowFactor, std::format("player.{}.slowFactor", typeInit.typeName));
						loadParam(typeInit.density, std::format("player.{}.density", typeInit.typeName));
						loadParam(typeInit.linearDamping, std::format("player.{}.linearDamping", typeInit.typeName));
						loadParam(typeInit.dash, std::format("player.{}.dash", typeInit.typeName));
						loadParam(typeInit.coolingRate, std::format("player.{}.coolingRate", typeInit.typeName));
						loadParam(typeInit.minAimingAngle, std::format("player.{}.minAimingAngle", typeInit.typeName));
						typeInit.minAimingAngle = glm::radians(typeInit.minAimingAngle);
						loadParam(typeInit.weaponOffset, std::format("player.{}.weaponOffset", typeInit.typeName));
						loadPresentationParams(typeInit.presentation, std::format("player.{}", typeInit.typeName));
						loadAnimationParams(typeInit.animation, std::format("player.{}", typeInit.typeName));

						prevPlayerName = typeInit.typeName;
					}
				}
			};

			auto loadEnemyParams = [&]() {
				std::string prevEnemyName;
				for (const auto& [key, value] : keysToValues)
				{
					if (auto enemyName = extractName(key, "enemy."); !enemyName.empty() && enemyName != prevEnemyName)
					{
						auto& typeInit = enemyGameComponents.typeNamesToTypes.emplace(enemyName, EnemyType{}).first->second.init;
						typeInit.typeName = std::move(enemyName);

						loadParam(typeInit.hp, std::format("enemy.{}.hp", typeInit.typeName));
						loadParam(typeInit.radiusRange, std::format("enemy.{}.radiusRange", typeInit.typeName));
						loadParam(typeInit.density, std::format("enemy.{}.density", typeInit.typeName));
						loadParam(typeInit.baseVelocity, std::format("enemy.{}.baseVelocity", typeInit.typeName));
						loadParam(typeInit.boostDistance, std::format("enemy.{}.boostDistance", typeInit.typeName));
						loadParam(typeInit.boostFactor, std::format("enemy.{}.boostFactor", typeInit.typeName));
						loadParam(typeInit.slowFactor, std::format("enemy.{}.slowFactor", typeInit.typeName));
						loadParam(typeInit.radiusReductionFactor, std::format("enemy.{}.radiusReductionFactor", typeInit.typeName));
						loadParam(typeInit.minimalRadius, std::format("enemy.{}.minimalRadius", typeInit.typeName));
						loadParam(typeInit.killSpawns, std::format("enemy.{}.killSpawns", typeInit.typeName));
						loadParam(typeInit.coolingRate, std::format("enemy.{}.coolingRate", typeInit.typeName));
						loadParam(typeInit.weaponOffset, std::format("enemy.{}.weaponOffset", typeInit.typeName));
						loadPresentationParams(typeInit.presentation, std::format("enemy.{}", typeInit.typeName));
						loadAnimationParams(typeInit.animation, std::format("enemy.{}", typeInit.typeName));

						prevEnemyName = typeInit.typeName;
					}
				}
			};

			auto loadWeaponParams = [&]() {
				std::string prevWeaponName;
				for (const auto& [key, value] : keysToValues)
				{
					if (auto weaponName = extractName(key, "weapon."); !weaponName.empty() && weaponName != prevWeaponName)
					{
						auto& typeInit = weaponGameComponents.typeNamesToTypes.emplace(weaponName, WeaponType{}).first->second.init;
						typeInit.typeName = std::move(weaponName);

						loadParam(typeInit.archetype, std::format("weapon.{}.archetype", typeInit.typeName));
						loadParam(typeInit.aiming, std::format("weapon.{}.aiming", typeInit.typeName));
						loadParam(typeInit.targeting, std::format("weapon.{}.targeting", typeInit.typeName));
						loadParam(typeInit.distance, std::format("weapon.{}.distance", typeInit.typeName));
						loadParam(typeInit.damageFactor, std::format("weapon.{}.damageFactor", typeInit.typeName));
						loadParam(typeInit.overchargingRate, std::format("weapon.{}.overchargingRate", typeInit.typeName));
						loadParam(typeInit.reloadTime, std::format("weapon.{}.reloadTime", typeInit.typeName));
						loadParam(typeInit.shotDuration, std::format("weapon.{}.shotDuration", typeInit.typeName), false);
						loadParam(typeInit.multishot, std::format("weapon.{}.multishot", typeInit.typeName));
						loadParam(typeInit.velocity, std::format("weapon.{}.velocity", typeInit.typeName), false);

						prevWeaponName = typeInit.typeName;
					}
				}
			};

			gameParams = {};
			loadParam(gameParams.pixelArt, "game.pixelArt", false);
			loadParam(gameParams.globalVolume, "game.globalVolume");
			loadParam(gameParams.musicVolume, "game.musicVolume");
			loadParam(gameParams.musicFile, "game.musicFile");
			loadParam(gameParams.camera.minProjectionHSize, "game.camera.minProjectionHSize");
			loadParam(gameParams.camera.trackingMargin, "game.camera.trackingMargin");
			loadParam(gameParams.camera.positionTransitionFactor, "game.camera.positionTransitionFactor");
			loadParam(gameParams.camera.projectionTransitionFactor, "game.camera.projectionTransitionFactor");
			loadParam(gameParams.gamepad.firstPlayer, "game.gamepad.firstPlayer");
			loadParam(gameParams.gamepad.deadZone, "game.gamepad.deadZone");
			loadParam(gameParams.gamepad.triggerDeadZone, "game.gamepad.triggerDeadZone");

			levelParams = {};
			loadParam(levelParams.backgroundTexture, "level.backgroundTexture");
			loadParam(levelParams.mapHHeight, "level.mapHHeight");
			loadParam(levelParams.walls.boxes, "level.walls.box", false);
			loadParam(levelParams.walls.circles, "level.walls.circle", false);
			loadParam(levelParams.debris.count, "level.debris.count", false);
			loadParam(levelParams.debris.hWidthRange, "level.debris.hWidthRange", false);
			loadParam(levelParams.debris.hHeightRatioRange, "level.debris.hHeightRatioRange", false);
			loadParam(levelParams.debris.radiusRange, "level.debris.radiusRange", false);
			loadParam(levelParams.debris.angleRange, "level.debris.angleRange", false);
			levelParams.debris.angleRange = { glm::radians(levelParams.debris.angleRange.x), glm::radians(levelParams.debris.angleRange.y) };
			loadParam(levelParams.debris.density, "level.debris.density", false);

			playerGameComponents.reset();
			enemyGameComponents.reset();
			weaponGameComponents.reset();

			loadPlayerParams();
			loadEnemyParams();
			loadWeaponParams();

			actions = {};
			loadParam(actions.initSpawners, "action.spawn", false);
		}

		void reload(bool loadParams = true)
		{
			if (loadParams)
				this->loadParams();

			auto& musics = Globals::Components().musics();
			auto& audioListener = Globals::Components().audioListener();

			const auto musicPath = std::format("audio/{}", gameParams.musicFile.substr(1, gameParams.musicFile.length() - 2));
			if (musics.empty() || musics.begin()->getPath() != musicPath)
			{
				for (auto& music : musics)
					music.state = ComponentState::Outdated;

				musics.emplace(musicPath).play();
				musicId = musics.last().getComponentId();
			}

			audioListener.setVolume(gameParams.globalVolume);
			musics[musicId].setVolume(gameParams.musicVolume);

			playerGameComponents.resetInstances();
			enemyGameComponents.resetInstances();
			weaponGameComponents.resetInstances();

			shockwavesToDamage.clear();
			mousePos = {};

			for (auto explosionId : playerSourceExplosions)
				Globals::Components().shockwaves()[explosionId].state = ComponentState::Outdated;
			playerSourceExplosions.clear();

			for (auto explosionId : enemySourceExplosions)
				Globals::Components().shockwaves()[explosionId].state = ComponentState::Outdated;
			enemySourceExplosions.clear();

			for (auto fireballId : fireballs)
				Globals::Components().actors()[fireballId].state = ComponentState::Outdated;
			fireballs.clear();

			for (auto& hordeSpawner : actions.initSpawners)
				for (int i = 0; i < hordeSpawner.count; ++i)
					std::visit([&](auto& actorType) {
						static constexpr bool playerType = std::is_same_v<std::remove_cvref_t<decltype(actorType)>, std::pair<PlayerType*, int>>;

						if constexpr (playerType)
							playerSpawn(*actorType.first, actorType.second, hordeSpawner.weaponTypes, hordeSpawner.position + glm::circularRand(0.1f));
						else
							enemySpawn(*actorType, hordeSpawner.weaponTypes, hordeSpawner.position + glm::circularRand(0.1f),
								glm::linearRand(actorType->init.radiusRange.x, actorType->init.radiusRange.y));
					}, hordeSpawner.actorTypeVariant);
		}

		struct SoundsLimitters;

		constexpr SoundsLimitters& soundLimitters(bool playerType)
		{
			return playerType
				? playerSoundLimitters
				: enemieSoundLimitters;
		}

		const Tools::BodyParams defaultBodyParams = Tools::BodyParams{}
			.linearDamping(6.0f)
			.fixedRotation(true);

		ComponentId backgroundTextureId{};
		ComponentId coffinTextureId{};
		ComponentId embryoTextureId{};
		ComponentId fogTextureId{};
		ComponentId explosionTextureId{};
		ComponentId fireballTextureId{};
		ComponentId jetfireAnimationTextureId{};

		ComponentId jetfireAnimatedTextureId{};

		ComponentId musicId{};

		ComponentId sparkingSoundBufferId{};
		ComponentId overchargedSoundBufferId{};
		ComponentId dashSoundBufferId{};
		ComponentId enemyKillSoundBufferId{};
		ComponentId explosionSoundBufferId{};
		ComponentId thrustSoundBufferId{};
		ComponentId thunderSoundBufferId{};

		GameParams gameParams{};
		LevelParams levelParams{};
		Actions actions{};

		GameComponents<WeaponType> weaponGameComponents;
		GameComponents<PlayerType> playerGameComponents;
		GameComponents<EnemyType> enemyGameComponents;

		std::vector<std::function<void()>> deferredWeaponsSteps;
		std::vector<std::function<void()>> deferredWeaponsPostSteps;
		std::vector<std::function<void()>> postSteps;

		std::unordered_set<ComponentId> playerSourceExplosions;
		std::unordered_set<ComponentId> enemySourceExplosions;
		std::unordered_map<ComponentId, float> shockwavesToDamage;
		std::unordered_set<ComponentId> fireballs;

		struct SoundsLimitters
		{
			std::shared_ptr<Tools::SoundsLimitter> weapons = Tools::SoundsLimitter::create(8);
			std::shared_ptr<Tools::SoundsLimitter> overcharges = Tools::SoundsLimitter::create(8);
			std::shared_ptr<Tools::SoundsLimitter> dashes = Tools::SoundsLimitter::create(8);
			std::shared_ptr<Tools::SoundsLimitter> kills = Tools::SoundsLimitter::create(8);
		} playerSoundLimitters, enemieSoundLimitters;

		std::vector<int> shuffledTargetSeqsInRange;
		std::unordered_map<int, int> shuffledInRangeTargetSeqsMapping;

		glm::vec2 mousePos{};

		struct
		{
			bool presentationTransparency = true;
			bool hitboxesRendering = false;
		} debug;

		bool bonusLevel = false;
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
