#pragma once

#include <components/_componentId.hpp>

#include <ogl/shaders/particles.hpp>

#include <commonTypes/resolutionMode.hpp>

#include <glm/vec2.hpp>

#include <functional>

namespace Shaders
{
	namespace Programs
	{
		struct Julia;
	}
}

namespace Tools
{
	struct PlaneHandler
	{
		size_t planeId = 0;
		size_t backThrustsIds[2] = {};
	};

	struct MissileHandler
	{
		MissileHandler();
		MissileHandler(ComponentId missileId, ComponentId backThrustId, glm::vec2 referenceVelocity);
		~MissileHandler();
		MissileHandler(MissileHandler&& other) noexcept;
		MissileHandler& operator=(MissileHandler&& other) noexcept;

		ComponentId missileId = 0;
		ComponentId backThrustId = 0;
		glm::vec2 referenceVelocity{};

	private:
		bool valid = true;
	};

	struct ExplosionParams
	{
		ExplosionParams& center(glm::vec2 value)
		{
			center_ = value;
			return *this;
		}

		ExplosionParams& numOfParticles(int value)
		{
			numOfParticles_ = value;
			return *this;
		}

		ExplosionParams& initVelocity(float value)
		{
			initVelocity_ = value;
			return *this;
		}

		ExplosionParams& particlesRadius(float value)
		{
			particlesRadius_ = value;
			return *this;
		}

		ExplosionParams& particlesDensity(float value)
		{
			particlesDensity_ = value;
			return *this;
		}

		ExplosionParams& particlesLinearDamping(float value)
		{
			particlesLinearDamping_ = value;
			return *this;
		}

		ExplosionParams& particlesAsBullets(bool value)
		{
			particlesAsBullets_ = value;
			return *this;
		}

		ExplosionParams& explosionDuration(float value)
		{
			explosionDuration_ = value;
			return *this;
		}

		ExplosionParams& explosionTexture(unsigned value)
		{
			explosionTexture_ = value;
			return *this;
		}

		ExplosionParams& particlesPerDecoration(int value)
		{
			particlesPerDecoration_ = value;
			return *this;
		}

		ExplosionParams& resolutionMode(ResolutionMode value)
		{
			resolutionMode_ = value;
			return *this;
		}

		glm::vec2 center_ = { 0, 0 };
		int numOfParticles_ = 64;
		float initVelocity_ = 100.0f;
		float particlesRadius_ = 1.0f;
		float particlesDensity_ = 0.01f;
		float particlesLinearDamping_ = 3.0f;
		bool particlesAsBullets_ = false;
		float explosionDuration_ = 1.0f;
		unsigned explosionTexture_ = 0;
		int particlesPerDecoration_ = 4;
		ResolutionMode resolutionMode_ = ResolutionMode::Normal;
	};

	PlaneHandler CreatePlane(unsigned planeTexture, unsigned flameAnimatedTexture, glm::vec2 position = glm::vec2(0.0f), float angle = 0.0f);
	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 referenceVelocity,
		glm::vec2 initialVelocity, unsigned missileTexture, unsigned flameAnimatedTexture);
	void CreateExplosion(ExplosionParams params);
	void CreateFogForeground(int numOfLayers, float alphaPerLayer, unsigned fogTexture,
		std::function<glm::vec4()> fColor = []() { return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); });
	void CreateJuliaBackground(std::function<glm::vec2()> juliaCOffset);
}
