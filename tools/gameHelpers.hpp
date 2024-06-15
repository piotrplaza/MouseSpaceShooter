#pragma once

#include "b2Helpers.hpp"

#include <commonTypes/componentMappers.hpp>
#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/componentId.hpp>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <functional>
#include <optional>
#include <list>
#include <memory>

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

	struct PlaneParams
	{
		PlaneParams& position(glm::vec2 value)
		{
			position_ = value;
			return *this;
		}

		PlaneParams& angle(float value)
		{
			angle_ = value;
			return *this;
		}

		PlaneParams& numOfThrusts(int value)
		{
			numOfThrusts_ = value;
			return *this;
		}

		PlaneParams& thrustAngle(float value)
		{
			thrustAngle_ = value;
			return *this;
		}

		PlaneParams& thrustOffset(glm::vec2 value)
		{
			thrustOffset_ = value;
			return *this;
		}

		PlaneParams& collisionBoxRendering(bool value)
		{
			collisionBoxRendering_ = value;
			return *this;
		}

		glm::vec2 position_ = { 0.0f, 0.0f };
		float angle_ = 0.0f;
		int numOfThrusts_ = 2;
		float thrustAngle_ = 0.1f;
		glm::vec2 thrustOffset_ = { -0.5f, 0.4f };
		bool collisionBoxRendering_ = false;
	};

	struct JuliaParams
	{
		JuliaParams() = default; // Functors calls result in random issues without this, prob due to some bug in the compiler.

		JuliaParams& juliaCF(std::function<glm::vec2()> value)
		{
			juliaCF_ = std::move(value);
			return *this;
		}

		JuliaParams& juliaCOffsetF(std::function<glm::vec2()> value)
		{
			juliaCOffsetF_ = std::move(value);
			return *this;
		}

		JuliaParams& minColorF(std::function<glm::vec4()> value)
		{
			minColorF_ = std::move(value);
			return *this;
		}

		JuliaParams& maxColorF(std::function<glm::vec4()> value)
		{
			maxColorF_ = std::move(value);
			return *this;
		}

		JuliaParams& iterationsF(std::function<int()> value)
		{
			iterationsF_ = std::move(value);
			return *this;
		}

		std::function<glm::vec2()> juliaCF_ = []() { return glm::vec2{ -0.1f, 0.65f }; };
		std::function<glm::vec2()> juliaCOffsetF_ = []() { return glm::vec2{ 0.0f }; };
		std::function<glm::vec4()> minColorF_ = []() { return glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }; };
		std::function<glm::vec4()> maxColorF_ = []() { return glm::vec4{ 0.0f, 0.2f, 0.1f, 1.0f }; };
		std::function<int()> iterationsF_ = []() { return 100; };
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

	class SoundsLimitter: public std::enable_shared_from_this<SoundsLimitter>
	{
	public:
		static std::shared_ptr<SoundsLimitter> create(unsigned limit);

		void newSound(Components::Sound& sound);

	private:
		SoundsLimitter(unsigned limit);

		const unsigned limit;
		std::list<Components::Sound*> sounds;
	};

	const Tools::BodyParams& GetDefaultParamsForPlaneBody();
	ComponentId CreatePlane(Body body, ComponentId planeTexture, ComponentId thrustAnimatedTexture, PlaneParams params = PlaneParams());
	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 referenceVelocity,
		glm::vec2 initialVelocity, ComponentId missileTexture, ComponentId thrustAnimatedTexture, std::optional<ComponentId> planeId = std::nullopt,
		std::optional<ComponentId> missileSoundBuffer = std::nullopt);
	void CreateExplosion(ExplosionParams params);
	void CreateFogForeground(int numOfLayers, float alphaPerLayer, ComponentId fogTexture,
		std::function<glm::vec4()> fColor = []() { return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); });
	void CreateJuliaBackground(JuliaParams params);
	Components::Sound& CreateAndPlaySound(ComponentId soundBuffer, std::function<glm::vec2()> posF = nullptr,
		std::function<void(Components::Sound&)> config = nullptr, std::function<void(Components::Sound&)> stepF = nullptr);
	std::function<void(b2Fixture&, b2Fixture&)> SkipDuplicatedBodiesCollisions(std::function<void(b2Fixture&, b2Fixture&)> handler);
}
