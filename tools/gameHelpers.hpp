#pragma once

#include <commonTypes/typeComponentMappers.hpp>
#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/componentId.hpp>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <functional>
#include <optional>
#include <list>

class b2Body;
class b2Fixture;

namespace Shaders
{
	namespace Programs
	{
		struct Julia;
	}
}

namespace Components
{
	struct Sound;
}

namespace Tools
{
	struct ExplosionParams
	{
		ExplosionParams& center(glm::vec2 value)
		{
			center_ = value;
			return *this;
		}

		ExplosionParams& sourceVelocity(glm::vec2 value)
		{
			sourceVelocity_ = value;
			return *this;
		}

		ExplosionParams& numOfParticles(int value)
		{
			numOfParticles_ = value;
			return *this;
		}

		ExplosionParams& initExplosionVelocity(float value)
		{
			initExplosionVelocity_ = value;
			return *this;
		}

		ExplosionParams& initExplosionVelocityRandomMinFactor(float value)
		{
			initExplosionVelocityRandomMinFactor_ = value;
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

		ExplosionParams& explosionTexture(ComponentId value)
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

		glm::vec2 center_ = { 0.0f, 0.0f };
		glm::vec2 sourceVelocity_ = { 0.0f, 0.0f };
		int numOfParticles_ = 64;
		float initExplosionVelocity_ = 100.0f;
		float initExplosionVelocityRandomMinFactor_ = 0.5f;
		float particlesRadius_ = 1.0f;
		float particlesDensity_ = 0.01f;
		float particlesLinearDamping_ = 3.0f;
		bool particlesAsBullets_ = false;
		float explosionDuration_ = 1.0f;
		ComponentId explosionTexture_ = 0;
		int particlesPerDecoration_ = 4;
		ResolutionMode resolutionMode_ = ResolutionMode::Normal;
	};

	struct MissileHandler
	{
		MissileHandler();
		MissileHandler(ComponentId missileId, ComponentId backThrustId, glm::vec2 referenceVelocity, std::optional<ComponentId> planeId = std::nullopt,
			std::optional<ComponentId> soundId = std::nullopt);
		MissileHandler(MissileHandler&& other) noexcept;

		~MissileHandler();

		MissileHandler& operator=(MissileHandler&& other) noexcept;

		ComponentId missileId = 0;
		ComponentId backThrustId = 0;
		glm::vec2 referenceVelocity{};
		std::optional<ComponentId> planeId;
		std::optional<ComponentId> soundId;

	private:
		bool valid = true;
	};

	class SoundsLimitter
	{
	public:
		SoundsLimitter(unsigned limit);
		~SoundsLimitter();

		void newSound(Components::Sound& sound);

	private:
		const unsigned limit;
		std::list<Components::Sound*> sounds;
	};

	ComponentId CreatePlane(ComponentId planeTexture, ComponentId flameAnimatedTexture, glm::vec2 position = glm::vec2(0.0f), float angle = 0.0f);
	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 referenceVelocity,
		glm::vec2 initialVelocity, ComponentId missileTexture, ComponentId flameAnimatedTexture, std::optional<ComponentId> planeId = std::nullopt,
		std::optional<ComponentId> missileSoundBuffer = std::nullopt);
	void CreateExplosion(ExplosionParams params);
	void CreateFogForeground(int numOfLayers, float alphaPerLayer, ComponentId fogTexture,
		std::function<glm::vec4()> fColor = []() { return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); });
	void CreateJuliaBackground(std::function<glm::vec2()> juliaCOffset);
	glm::vec2 GetRelativePos(glm::vec2 scenePos, bool projectionSizeScaling = true);
	Components::Sound& PlaySingleSound(ComponentId soundBuffer, std::function<glm::vec2()> posF = nullptr,
		std::function<void(Components::Sound&)> config = nullptr, std::function<void(Components::Sound&)> stepF = nullptr);
	std::function<void(b2Fixture&, b2Fixture&)> SkipDuplicatedBodiesCollisions(std::function<void(b2Fixture&, b2Fixture&)> handler);
}
