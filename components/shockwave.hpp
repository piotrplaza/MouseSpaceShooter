#pragma once

#include "_componentBase.hpp"

#include <tools/b2Helpers.hpp>
#include <tools/utility.hpp>

#include <globals/collisionBits.hpp>

#include <Box2D/Box2D.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <memory>
#include <vector>

namespace Components
{
	struct Shockwave : ComponentBase
	{
		Shockwave(glm::vec2 center, glm::vec2 sourceVelocity, int numOfParticles, float initExplosionVelocity, float initExplosionVelocityRandomMinFactor,
			float particlesRadius, float particlesDensity, float particlesLinearDamping, bool particlesAsBullets, bool particlesAsSensors):
			center(center)
		{
			float angle = Tools::RandomFloat(0.0f, glm::two_pi<float>());
			const float angleStep = glm::two_pi<float>() / numOfParticles;
			for (int i = 0; i < numOfParticles; ++i)
			{
				particles.push_back(Tools::CreateCircleBody(particlesRadius,
					Tools::BodyParams().position(center).bodyType(b2_dynamicBody).density(particlesDensity).sensor(particlesAsSensors)));
				particles.back()->SetBullet(particlesAsBullets);
				particles.back()->SetLinearVelocity(ToVec2<b2Vec2>(sourceVelocity + glm::vec2(glm::cos(angle), glm::sin(angle)) * initExplosionVelocity *
					Tools::RandomFloat(initExplosionVelocityRandomMinFactor, 1.0f)));
				particles.back()->SetLinearDamping(particlesLinearDamping);
				angle += angleStep;
			}
		}

		const glm::vec2 center;
		std::vector<Body> particles;

		void init(ComponentId id, bool static_) override
		{
			ComponentBase::init(id, static_);
			for (auto& particle : particles)
			{
				Tools::SetCollisionFilteringBits(*particle, Globals::CollisionBits::shockwaveParticle,
					Globals::CollisionBits::all - Globals::CollisionBits::shockwaveParticle - Globals::CollisionBits::projectile);
				Tools::AccessUserData(*particle).bodyComponentVariant = CM::ShockwaveParticle(*this);
			}
		}

		void setEnabled(bool value) override
		{
			ComponentBase::setEnabled(value);
			for (auto& particle : particles)
				particle->SetEnabled(false);
		}
	};
}
