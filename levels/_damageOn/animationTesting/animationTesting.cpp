#include "animationTesting.hpp"

#include <components/camera2D.hpp>
#include <components/keyboard.hpp>
#include <components/gamepad.hpp>
#include <components/texture.hpp>
#include <components/animatedTexture.hpp>
#include <components/actor.hpp>
#include <components/wall.hpp>
#include <components/decoration.hpp>
#include <components/music.hpp>
#include <globals/components.hpp>

#include <tools/shapes2D.hpp>
#include <tools/utility.hpp>

#include <ogl/uniformsUtils.hpp>

#include <glm/gtc/random.hpp>

#include <array>
#include <unordered_map>
#include <fstream>
#include <iostream>

namespace Levels::DamageOn
{
	namespace
	{
		constexpr float mapHSize = 15.0f;
		constexpr float enemyRadius = 0.5f;
		constexpr float enemyMaxVelocity = 2.0f;
		constexpr float enemyDensity = 1.0f;
		constexpr int enemyCount = 100;
		constexpr float debrisDensity = 100.0f;
		constexpr float gamepadDeadZone = 0.1f;

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
			auto& musics = Globals::Components().musics();
			musics.emplace("audio/Damage On.ogg", 0.8f).play();

			auto& textures = Globals::Components().staticTextures();
			auto& animatedTextures = Globals::Components().staticAnimatedTextures();

			backgroundTextureId = textures.emplace("textures/damageOn/cemetary.jpg", GL_CLAMP_TO_BORDER).getComponentId();
			textures.last().magFilter = GL_NEAREST;
			//textures.last().preserveAspectRatio = true;

			greenMarbleTextureId = textures.emplace("textures/green marble.jpg", GL_MIRRORED_REPEAT).getComponentId();
			textures.last().magFilter = GL_NEAREST;
			textures.last().scale = glm::vec2(30.0f);
			textures.last().preserveAspectRatio = true;

			coffinTextureId = textures.emplace("textures/damageOn/coffin.png", GL_MIRRORED_REPEAT).getComponentId();
			textures.last().magFilter = GL_NEAREST;
			//textures.last().scale = glm::vec2(30.0f);

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

			playerAnimatedTextureId = animatedTextures.add({ CM::StaticTexture(playerAnimationTextureId), { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
				AnimationDirection::Forward, AnimationPolicy::Repeat, TextureLayout::Horizontal }).getComponentId();
			animatedTextures.last().start(true);

			for (auto& enemyAnimatedTextureId : enemyAnimatedTextureIds)
			{
				enemyAnimatedTextureId = animatedTextures.add({ CM::StaticTexture(enemyAnimationTextureId), { 263, 525 }, { 5, 10 }, { 0, 0 }, 210, 473, { 52, 52 }, 0.02f, 50, Tools::RandomInt(0, 49),
					AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal }).getComponentId();
				animatedTextures.last().start(true);
			}

			reload();
		}

		void postSetup()
		{
			auto& camera = Globals::Components().camera2D();
			auto& actors = Globals::Components().actors();
			auto& walls = Globals::Components().staticWalls();
			auto& decorations = Globals::Components().staticDecorations();
			auto& textures = Globals::Components().staticTextures();

			const glm::vec2 levelHSize(textures[backgroundTextureId].loaded.getAspectRatio() * mapHSize, mapHSize);
			camera.targetProjectionHSizeF = camera.details.projectionHSize = camera.details.prevProjectionHSize = playerRadius * 8.0f;
			camera.targetPositionF = [&, levelHSize]() { return glm::clamp(actors[playerId].getOrigin2D(), -levelHSize + camera.details.completeProjectionHSize, levelHSize - camera.details.completeProjectionHSize); };
			camera.positionTransitionFactor = 5.0f;

			const float borderHThickness = 10.0f;
			walls.emplace(Tools::CreateBoxBody({ levelHSize.x + borderHThickness, borderHThickness }, Tools::BodyParams{}.position({ 0.0f, -levelHSize.y - borderHThickness }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			walls.emplace(Tools::CreateBoxBody({ levelHSize.x + borderHThickness, borderHThickness }, Tools::BodyParams{}.position({ 0.0f, levelHSize.y + borderHThickness }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			walls.emplace(Tools::CreateBoxBody({ borderHThickness, levelHSize.y + borderHThickness }, Tools::BodyParams{}.position({ -levelHSize.x - borderHThickness, 0.0f }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			walls.emplace(Tools::CreateBoxBody({ borderHThickness, levelHSize.y + borderHThickness }, Tools::BodyParams{}.position({ levelHSize.x + borderHThickness, 0.0f }))).colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			auto screenCordTexturesF = [sceneCoordTextures = UniformsUtils::Uniform1b()](ShadersUtils::ProgramId program) mutable {
				if (!sceneCoordTextures.isValid())
					sceneCoordTextures = UniformsUtils::Uniform1b(program, "sceneCoordTextures");
				sceneCoordTextures(true);
				return [=]() mutable { sceneCoordTextures(false); };
			};

			for (auto sign : { -1, 1 })
				walls.emplace(Tools::CreateBoxBody(glm::vec2(2.0f), Tools::BodyParams{}.position({ sign * levelHSize.x / 2.5f, 0.0f })), CM::StaticTexture(greenMarbleTextureId)).renderingSetupF = screenCordTexturesF;

			for (int i = 0; i < 20; ++i)
			{
				const float debrisWidth = glm::linearRand(0.3f, 1.0f);
				const float debrisHeight = debrisWidth * glm::linearRand(1.6f, 2.0f);
				walls.emplace(Tools::CreateBoxBody({ debrisWidth, debrisHeight }, Tools::BodyParams{}.position(glm::linearRand(-levelHSize, levelHSize))
					.bodyType(b2_dynamicBody).linearDamping(10.0f).angularDamping(10.0f).density(debrisDensity)), CM::StaticTexture(coffinTextureId));
				walls.last().texCoord = Shapes2D::CreateTexCoordOfRectangle();
			}

			for (int i = 0; i < enemyCount; ++i)
			{
				enemyIds[i] = actors.emplace(Tools::CreateCircleBody(enemyRadius, Tools::BodyParams{ defaultBodyParams }
					.bodyType(b2_dynamicBody)
					.density(enemyDensity)
					.position(glm::linearRand(-levelHSize, levelHSize))), CM::StaticAnimatedTexture(enemyAnimatedTextureIds[i], {}, {}, { 1.3f, 1.3f }))
					.getComponentId();
				actors.last().renderingSetupF = createRecursiveFaceRS({ enemyRadius * 0.6f, enemyRadius });
				actors.last().colorF = glm::vec4(glm::vec3(glm::linearRand(0.0f, 1.0f)), 1.0f) * 0.8f;
			}

			decorations.emplace(Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, levelHSize), CM::StaticTexture(backgroundTextureId), Shapes2D::CreateTexCoordOfRectangle()).renderLayer = RenderLayer::FarBackground;
		}

		void step()
		{
			auto& keyboard = Globals::Components().keyboard();
			auto& gamepad = Globals::Components().gamepads()[0];
			auto& player = Globals::Components().actors()[playerId];

			for (auto enemyId : enemyIds)
			{
				auto& enemy = Globals::Components().actors()[enemyId];
				const auto direction = player.getOrigin2D() - enemy.getOrigin2D();
				const auto directionLength = glm::length(direction);
				if (directionLength > 0.1f)
					enemy.setVelocity(direction / directionLength * enemyMaxVelocity);
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

			const glm::vec2 newVelocity = direction * playerMaxVelocity;
			if (glm::length(newVelocity) > glm::length(player.getVelocity()))
				player.setVelocity(newVelocity);

			if (keyboard.pressed[' '] || gamepad.pressed.a)
				player.body->ApplyLinearImpulseToCenter(ToVec2<b2Vec2>(direction * playerDash * player.body->GetMass()), true);

			if (keyboard.pressed['T'] || gamepad.pressed.y)
				playerTransparency = !playerTransparency;
			if (keyboard.pressed['B'] || gamepad.pressed.x)
				playerBodyRendering = !playerBodyRendering;
			if (keyboard.pressed['P'] || gamepad.pressed.start)
				reload();
		}

	private:
		void reload()
		{
			std::ifstream file("levels/_damageOn/animationTesting/params.txt");
			if (!file.is_open())
				std::cout << "unable to open \"levels/_damageOn/animationTesting/params.txt\"" << std::endl;

			std::string key, value;
			while (file >> key >> value)
				params[key] = value;

			std::cout << params.size() << std::endl;
			for (const auto& [key, value] : params)
				std::cout << key << " " << value << std::endl;

			playerRadius = std::stof(params["player.radius"]);
			playerMaxVelocity = std::stof(params["player.maxVelocity"]);
			playerDensity = std::stof(params["player.density"]);
			playerLinearDamping = std::stof(params["player.linearDamping"]);
			playerDash = std::stof(params["player.dash"]);

			playerPresentationRadiusProportions = { std::stof(params["player.presentation.radiusProportions.x"]), std::stof(params["player.presentation.radiusProportions.y"]) };
			playerPresentationScale = { std::stof(params["player.presentation.scale.x"]), std::stof(params["player.presentation.scale.y"]) };

			auto& actors = Globals::Components().actors();
			const glm::vec2 playerPresentationSize = playerPresentationRadiusProportions * playerRadius;

			auto reloadPlayer = [&]() {
				auto& player = actors[playerId];
				player.changeBody(Tools::CreateCircleBody(playerRadius, Tools::BodyParams{}.linearDamping(playerLinearDamping).fixedRotation(true).bodyType(b2_dynamicBody).density(playerDensity).position(player.getOrigin2D())));
				player.subsequence.back().vertices = Shapes2D::CreateVerticesOfRectangle({ 0.0f, playerPresentationSize.y - playerRadius }, playerPresentationSize);
				player.subsequence.back().texture = CM::StaticAnimatedTexture(playerAnimatedTextureId, { 0.0f, playerPresentationSize.y - playerRadius }, {}, playerPresentationSize * playerPresentationScale);
				player.subsequence.back().modelMatrixF = player.modelMatrixF;
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
				reloadPlayer();
			}
			else
				reloadPlayer();
		}

		const Tools::BodyParams defaultBodyParams = Tools::BodyParams{}
			.linearDamping(6.0f)
			.fixedRotation(true);

		ComponentId playerId{};
		std::array<ComponentId, enemyCount> enemyIds{};

		ComponentId backgroundTextureId{};
		ComponentId greenMarbleTextureId{};
		ComponentId coffinTextureId{};

		ComponentId playerAnimationTextureId{};
		ComponentId enemyAnimationTextureId{};

		ComponentId playerAnimatedTextureId{};
		std::array<ComponentId, enemyCount> enemyAnimatedTextureIds{};

		bool playerTransparency = true;
		bool playerBodyRendering = false;

		std::unordered_map<std::string, std::string> params;
		float playerRadius{};
		float playerMaxVelocity{};
		float playerDensity{};
		float playerLinearDamping{};
		float playerDash{};
		glm::vec2 playerPresentationRadiusProportions{};
		glm::vec2 playerPresentationScale{};
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
