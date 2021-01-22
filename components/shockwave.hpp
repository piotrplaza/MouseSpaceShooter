#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <Box2D/Box2D.h>

#include <componentBase.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/utility.hpp>

#include <collisionBits.hpp>

namespace Components
{
	struct Shockwave : ComponentBase
	{
		Shockwave(glm::vec2 center, int numOfParticles = 100, float initVelocity = 100.0f, float particlesRadius = 0.5f, float particlesDensity = 0.01f,
			float particlesLinearDamping = 3.0f, bool particlesAsBullets = false)
		{
			float angle = Tools::Random(0.0f, glm::two_pi<float>());
			const float angleStep = glm::two_pi<float>() / numOfParticles;
			for (int i = 0; i < numOfParticles; ++i)
			{
				particles.push_back(Tools::CreateCircleBody(center, particlesRadius, b2_dynamicBody, particlesDensity));
				particles.back()->SetBullet(particlesAsBullets);
				particles.back()->SetLinearVelocity(b2Vec2(glm::cos(angle), glm::sin(angle)) * initVelocity);
				particles.back()->SetLinearDamping(particlesLinearDamping);
				Tools::SetCollisionFilteringBits(*particles.back(), CollisionBits::shockwaveParticleBit,
					CollisionBits::all - CollisionBits::shockwaveParticleBit - CollisionBits::missileBit);
				angle += angleStep;
			}
		}

		std::vector<Body> particles;
	};
}
