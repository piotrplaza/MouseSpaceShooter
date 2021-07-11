#pragma once

#include <functional>

#include <glm/vec2.hpp>

#include <componentId.hpp>

#include <ogl/shaders/particles.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct Julia;
	}
}

namespace Tools
{
	struct PlayerPlaneHandler
	{
		size_t playerId = 0;
		size_t backThrustsIds[2] = {};
	};

	struct MissileHandler
	{
		MissileHandler();
		MissileHandler(ComponentId missileId, ComponentId backThrustId);
		~MissileHandler();
		MissileHandler(MissileHandler&& other) noexcept;
		MissileHandler& operator=(MissileHandler&& other) noexcept;

		ComponentId missileId = 0;
		ComponentId backThrustId = 0;

	private:
		bool valid = true;
	};

	PlayerPlaneHandler CreatePlayerPlane(unsigned planeTexture, unsigned flameAnimationTexture);
	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 initialVelocity, unsigned missileTexture, unsigned flameAnimationTexture);
	void CreateExplosion(Shaders::Programs::ParticlesAccessor particlesProgram, glm::vec2 center, unsigned explosionTexture, float explosionDuration = 1.0f, int numOfParticles = 64, int particlesPerDecoration = 4);
	void CreateFogForeground(int numOfLayers, float alphaPerLayer, unsigned fogTexture);
	void CreateJuliaBackground(Shaders::Programs::Julia& juliaShaders, std::function<glm::vec2()> juliaCOffset);
}
