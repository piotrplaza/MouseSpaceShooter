#include "collisions.hpp"

#include <components/mouse.hpp>
#include <components/wall.hpp>
#include <components/texture.hpp>
#include <components/screenInfo.hpp>
#include <components/physics.hpp>

#include <globals/components.hpp>

#include <tools/utility.hpp>

namespace Levels
{
	class Collisions::Impl
	{
	public:
		void setup()
		{
			auto& physics = Globals::Components().physics();
			physics.world->SetGravity({0.0f, -50.0f});
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().staticTextures();

			dzidziaTexture = textures.size();
			textures.emplace("textures/photos/dzidzia1.png");
			textures.last().translate = glm::vec2(-0.1f, 0.0f);
			textures.last().scale = glm::vec2(3.5f, 3.9f);
			textures.last().minFilter = GL_LINEAR;
		}

		void createWalls()
		{
			auto& staticWalls = Globals::Components().staticWalls();
			auto& mouse = Globals::Components().mouse();
			auto& screenInfo = Globals::Components().screenInfo();

			staticWalls.emplace(Tools::CreateBoxBody({ 1.0f, 10.0f }, Tools::BodyParams().position({ -10.0f * screenInfo.getAspectRatio() - 1.0f, 0.0f })));
			staticWalls.emplace(Tools::CreateBoxBody({ 1.0f, 10.0f }, Tools::BodyParams().position({ 10.0f * screenInfo.getAspectRatio() + 1.0f, 0.0f })));
			staticWalls.emplace(Tools::CreateBoxBody({ 10.0f * screenInfo.getAspectRatio(), 1.0f }, Tools::BodyParams().position({ 0.0f, -11.0f })));
			staticWalls.emplace(Tools::CreateBoxBody({ 10.0f * screenInfo.getAspectRatio(), 1.0f }, Tools::BodyParams().position({ 0.0f, 11.0f })));

			staticWalls.emplace(Tools::CreateCircleBody(2.0f, Tools::BodyParams().bodyType(b2_kinematicBody).position({0.0f, 7.0f}).restitution(0.2f)), CM::StaticTexture(dzidziaTexture));
			staticWalls.last().stepF = [&wall = staticWalls.last(), &mouse]() {
				wall.setVelocity(mouse.getCartesianDelta() * 0.8f);
			};

#if 1
			for (int i = 0; i < 1000; ++i)
			{
				staticWalls.emplace(Tools::CreateCircleBody(Tools::RandomFloat(0.2f, 0.5f), Tools::BodyParams().bodyType(b2_dynamicBody)
					.position({ Tools::RandomFloat(-9.0f, 9.0f) * screenInfo.getAspectRatio(), Tools::RandomFloat(-9.0f, 9.0f) }).restitution(0.2f).linearDamping(0.1f)));
				staticWalls.last().colorF = [color = glm::vec4(Tools::RandomFloat(0.0f, 1.0f), Tools::RandomFloat(0.0f, 1.0f), Tools::RandomFloat(0.0f, 1.0f), 1.0f)]() { return color; };
			}
#else
			const glm::vec2 hSize(0.3f, 0.3f);
			const int height = 20;
			for (int i = 0; i < height; ++i)
			{
				for (int j = 0; j <= i; ++j)
				{
					const float startX = j * hSize.x - i * hSize.x;
					const float centerY = -9.5f + hSize.y * 2.0f * (height - i - 1);
					staticWalls.emplace(Tools::CreateBoxBody(hSize, Tools::BodyParams().bodyType(b2_dynamicBody)
						.position({ startX + j * hSize.x, centerY }).restitution(0.0f).friction(0.5f).autoSleeping(true).sleeping(true)));
					staticWalls.last().colorF = [color = glm::vec4(Tools::Random(0.0f, 1.0f), Tools::Random(0.0f, 1.0f), Tools::Random(0.0f, 1.0f), 1.0f)]() { return color; };
				}
			}
#endif
		}

		void step()
		{
		}

	private:
		unsigned dzidziaTexture = 0;
	};

	Collisions::Collisions() :
		impl(std::make_unique<Impl>())
	{
		impl->loadTextures();
		impl->setup();
		impl->createWalls();
	}

	Collisions::~Collisions() = default;

	void Collisions::step()
	{
		impl->step();
	}
}
