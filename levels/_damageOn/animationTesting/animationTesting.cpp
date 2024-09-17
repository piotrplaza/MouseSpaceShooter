#include "animationTesting.hpp"

#include <components/camera2D.hpp>
#include <components/keyboard.hpp>
#include <components/gamepad.hpp>
#include <components/texture.hpp>
#include <components/animatedTexture.hpp>
#include <components/actor.hpp>
#include <components/wall.hpp>
#include <components/decoration.hpp>
#include <globals/components.hpp>

#include <tools/shapes2D.hpp>
#include <tools/utility.hpp>

#include <ogl/uniformsUtils.hpp>

#include <glm/gtc/random.hpp>

#include <array>

namespace Levels::DamageOn
{
	namespace
	{
		constexpr float mapHSize = 10.0f;
		constexpr float playerRadius = 1.0f;
		constexpr float playerMaxVelocity = 10.0f;
		constexpr float enemyRadius = 0.5f;
		constexpr float enemyMaxVelocity = 2.0f;
		constexpr int enemyCount = 100;

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
			auto& camera = Globals::Components().camera2D();
			auto& textures = Globals::Components().staticTextures();
			auto& animatedTextures = Globals::Components().staticAnimatedTextures();

			camera.targetProjectionHSizeF = mapHSize;

			backgroundTextureId = textures.emplace("textures/damageOn/cemetary.jpg", GL_MIRRORED_REPEAT).getComponentId();

			wallTextureId = textures.emplace("textures/space rock.jpg", GL_MIRRORED_REPEAT).getComponentId();
			textures.last().scale = glm::vec2(5.0f);
			textures.last().preserveAspectRatio = true;

			playerAnimationTextureId = textures.emplace("textures/damageOn/player.png").getComponentId();
			textures.last().minFilter = GL_LINEAR;

			enemyAnimationTextureId = textures.emplace(TextureFile("textures/damageOn/enemy.jpg", 4, true, TextureFile::AdditionalConversion::DarkToTransparent, [](float* data, glm::ivec2 size, int numOfChannels) {
				for (int i = 0; i < size.x * size.y; ++i)
				{
					glm::vec4& pixel = reinterpret_cast<glm::vec4*>(data)[i];
					if (pixel.r + pixel.g + pixel.b < 0.2f)
						pixel = {};
				}
				})).getComponentId();
			textures.last().minFilter = GL_LINEAR;

			playerAnimatedTextureId = animatedTextures.add({ CM::StaticTexture(playerAnimationTextureId), { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
				AnimationDirection::Forward, AnimationPolicy::Repeat, TextureLayout::Horizontal }).getComponentId();
			animatedTextures.last().start(true);

			for (auto& enemyAnimatedTextureId : enemyAnimatedTextureIds) {
				enemyAnimatedTextureId = animatedTextures.add({ CM::StaticTexture(enemyAnimationTextureId), { 263, 525 }, { 5, 10 }, { 0, 0 }, 210, 473, { 52, 52 }, 0.02f, 50, Tools::RandomInt(0, 49),
					AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal }).getComponentId();
				animatedTextures.last().start(true);
			}
		}

		void postSetup()
		{
			auto& camera = Globals::Components().camera2D();
			auto& actors = Globals::Components().actors();
			auto& walls = Globals::Components().staticWalls();
			auto& decorations = Globals::Components().staticDecorations();
			const auto screenHSize = camera.details.completeProjectionHSize;

			camera.targetProjectionHSizeF = mapHSize;

			const float borderHThickness = 1.0f;
			walls.emplace(Tools::CreateBoxBody({ screenHSize.x + borderHThickness, borderHThickness }, Tools::BodyParams{}.position({ 0.0f, -screenHSize.y - borderHThickness })));
			walls.emplace(Tools::CreateBoxBody({ screenHSize.x + borderHThickness, borderHThickness }, Tools::BodyParams{}.position({ 0.0f, screenHSize.y + borderHThickness })));
			walls.emplace(Tools::CreateBoxBody({ borderHThickness, screenHSize.y + borderHThickness }, Tools::BodyParams{}.position({ -screenHSize.x - borderHThickness, 0.0f })));
			walls.emplace(Tools::CreateBoxBody({ borderHThickness, screenHSize.y + borderHThickness }, Tools::BodyParams{}.position({ screenHSize.x + borderHThickness, 0.0f })));

			auto screenCordTexturesF = [sceneCoordTextures = UniformsUtils::Uniform1b()](ShadersUtils::ProgramId program) mutable {
				if (!sceneCoordTextures.isValid())
					sceneCoordTextures = UniformsUtils::Uniform1b(program, "sceneCoordTextures");
				sceneCoordTextures(true);
				return [=]() mutable { sceneCoordTextures(false); };
			};

			for (auto sign : { -1, 1 })
				walls.emplace(Tools::CreateBoxBody(glm::vec2(mapHSize) / 8.0f, Tools::BodyParams{}.position({ sign * screenHSize.x / 2.0f, 0.0f })), CM::StaticTexture(wallTextureId)).renderingSetupF = screenCordTexturesF;

			const auto defaultBodyParams = Tools::BodyParams{}
				.linearDamping(10.0f)
				.fixedRotation(true);

			playerId = actors.emplace(Tools::CreateCircleBody(playerRadius, Tools::BodyParams{ defaultBodyParams }.bodyType(b2_dynamicBody).density(2.0f)), CM::StaticAnimatedTexture(playerAnimatedTextureId, {}, {}, {1.5f, 3.0f})).getComponentId();
			for (int i = 0; i < enemyCount; ++i)
			{
				enemyIds[i] = actors.emplace(Tools::CreateCircleBody(enemyRadius, Tools::BodyParams{ defaultBodyParams }
					.bodyType(b2_dynamicBody)
					.position(glm::linearRand(-screenHSize, screenHSize))), CM::StaticAnimatedTexture(enemyAnimatedTextureIds[i], {}, {}, { 1.3f, 1.3f }))
					.getComponentId();
				actors.last().renderingSetupF = createRecursiveFaceRS({ enemyRadius * 0.6f, enemyRadius });
				actors.last().colorF = glm::vec4(glm::vec3(glm::linearRand(0.0f, 1.0f)), 1.0f) * 0.8f;
			}

			decorations.emplace(Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, screenHSize), CM::StaticTexture(backgroundTextureId), Shapes2D::CreateTexCoordOfRectangle()).renderLayer = RenderLayer::FarBackground;
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

			glm::vec2 direction = gamepad.enabled ? gamepad.lStick : glm::vec2(0.0f);
			direction += glm::vec2(-(int)keyboard.pressing[/*VK_LEFT*/0x25] + (int)keyboard.pressing[/*VK_RIGHT*/0x27], -(int)keyboard.pressing[/*VK_DOWN*/0x28] + (int)keyboard.pressing[/*VK_UP*/0x26]);

			const float directionLength = glm::length(direction);
			if (directionLength > 0.1f)
			{
				if (directionLength > 1.0f && directionLength != 0.0f)
					direction /= directionLength;
				player.setVelocity(direction * playerMaxVelocity);
			}
		}

	private:
		ComponentId playerId;
		std::array<ComponentId, enemyCount> enemyIds;

		ComponentId backgroundTextureId;
		ComponentId wallTextureId;

		ComponentId playerAnimationTextureId;
		ComponentId enemyAnimationTextureId;

		ComponentId playerAnimatedTextureId;
		std::array<ComponentId, enemyCount> enemyAnimatedTextureIds;
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
