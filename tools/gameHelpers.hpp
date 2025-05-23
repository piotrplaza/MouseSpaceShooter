#pragma once

#include "b2Helpers.hpp"

#include <commonTypes/componentMappers.hpp>
#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/componentId.hpp>
#include <commonTypes/fTypes.hpp>
#include <commonTypes/renderLayer.hpp>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <functional>
#include <optional>
#include <list>
#include <memory>

class b2Body;
class b2Fixture;

namespace ShadersUtils
{
	namespace Programs
	{
		struct Julia;
	}
}

namespace Components
{
	struct Sound;
	struct Shockwave;
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

		ExplosionParams& particlesAsSensors(bool value)
		{
			particlesAsSensors_ = value;
			return *this;
		}

		ExplosionParams& deferredExectution(bool value)
		{
			deferredExectution_ = value;
			return *this;
		}

		ExplosionParams& explosionDuration(float value)
		{
			explosionDuration_ = value;
			return *this;
		}

		ExplosionParams& explosionTexture(CM::Texture value)
		{
			explosionTexture_ = std::move(value);
			return *this;
		}

		ExplosionParams& particlesPerDecoration(int value)
		{
			particlesPerDecoration_ = value;
			return *this;
		}

		ExplosionParams& presentationInitScale(float value)
		{
			presentationInitScale_ = value;
			return *this;
		}

		ExplosionParams& presentationScaleFactor(float value)
		{
			presentationScaleFactor_ = value;
			return *this;
		}

		ExplosionParams& color(glm::vec4 value)
		{
			color_ = value;
			return *this;
		}

		ExplosionParams& additiveBlending(bool value)
		{
			additiveBlending_ = value;
			return *this;
		}

		ExplosionParams& renderLayer(RenderLayer value)
		{
			renderLayer_ = value;
			return *this;
		}

		ExplosionParams& resolutionMode(ResolutionMode value)
		{
			resolutionMode_ = value;
			return *this;
		}

		ExplosionParams& beginCallback(std::function<void(Components::Shockwave&)> value)
		{
			beginCallback_ = std::move(value);
			return *this;
		}

		ExplosionParams& endCallback(std::function<void(Components::Shockwave&)> value)
		{
			endCallback_ = std::move(value);
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
		bool particlesAsSensors_ = false;
		bool deferredExectution_ = true;
		float explosionDuration_ = 1.0f;
		CM::Texture explosionTexture_;
		int particlesPerDecoration_ = 4;
		float presentationInitScale_ = 1.0f;
		float presentationScaleFactor_ = 20.0f;
		glm::vec4 color_ = glm::vec4(1.0f);
		bool additiveBlending_ = true;
		RenderLayer renderLayer_ = RenderLayer::FarForeground;
		ResolutionMode resolutionMode_{};
		std::function<void(Components::Shockwave&)> beginCallback_;
		std::function<void(Components::Shockwave&)> endCallback_;
	};

	struct SparkingParams
	{
		SparkingParams& sourcePoint(glm::vec2 value)
		{
			sourcePoint_ = value;
			return *this;
		}

		SparkingParams& initVelocity(glm::vec2 value)
		{
			initVelocity_ = value;
			return *this;
		}

		SparkingParams& initVelocityRandomMinFactor(float value)
		{
			initVelocityRandomMinFactor_ = value;
			return *this;
		}

		SparkingParams& gravity(glm::vec2 value)
		{
			gravity_ = value;
			return *this;
		}

		SparkingParams& spreadFactor(float value)
		{
			spreadFactor_ = value;
			return *this;
		}

		SparkingParams& sparksCount(int value)
		{
			sparksCount_ = value;
			return *this;
		}

		SparkingParams& trailsScale(float value)
		{
			trailsScale_ = value;
			return *this;
		}

		SparkingParams& lineWidth(float value)
		{
			lineWidth_ = value;
			return *this;
		}

		glm::vec2 sourcePoint_ = { 0.0f, 0.0f };
		glm::vec2  initVelocity_ = { 10.0f, 0.0f };
		float initVelocityRandomMinFactor_ = 0.01f;
		glm::vec2 gravity_ = { 0.0f, 0.0f };
		float spreadFactor_ = 1.0f;
		int sparksCount_ = 1000;
		float trailsScale_ = 1.0f;
		float lineWidth_ = 1.0f;
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
		JuliaParams& juliaCF(FVec2 value)
		{
			juliaCF_ = std::move(value);
			return *this;
		}

		JuliaParams& juliaCOffsetF(FVec2 value)
		{
			juliaCOffsetF_ = std::move(value);
			return *this;
		}

		JuliaParams& minColorF(FVec4 value)
		{
			minColorF_ = std::move(value);
			return *this;
		}

		JuliaParams& maxColorF(FVec4 value)
		{
			maxColorF_ = std::move(value);
			return *this;
		}

		JuliaParams& iterationsF(FInt value)
		{
			iterationsF_ = std::move(value);
			return *this;
		}

		FVec2 juliaCF_ = glm::vec2{ -0.1f, 0.65f };
		FVec2 juliaCOffsetF_ = glm::vec2{ 0.0f };
		FVec4 minColorF_ = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
		FVec4 maxColorF_ = glm::vec4{ 0.0f, 0.2f, 0.1f, 1.0f };
		FInt iterationsF_ = 100;
	};

	struct MissileHandler
	{
		MissileHandler();
		MissileHandler(CM::Missile missile, CM::Decoration backThrust, glm::vec2 referenceVelocity, std::optional<CM::Plane> plane = std::nullopt,
			std::optional<CM::Sound> sound = std::nullopt);
		MissileHandler(MissileHandler&& other) noexcept;

		~MissileHandler();

		MissileHandler& operator=(MissileHandler&& other) noexcept;

		CM::Missile missile;
		CM::Decoration backThrust;
		glm::vec2 referenceVelocity{};
		std::optional<CM::Plane> plane;
		std::optional<CM::Sound> sound;

	private:
		bool valid = true;
	};

	class SoundsLimitter: public std::enable_shared_from_this<SoundsLimitter>
	{
	public:
		static std::shared_ptr<SoundsLimitter> create(unsigned limit);

		Components::Sound& newSound(Components::Sound& sound, std::function<void()> tearDown = nullptr);

	private:
		SoundsLimitter(unsigned limit);

		const unsigned limit;
		std::list<Components::Sound*> sounds;
	};

	const Tools::BodyParams& GetDefaultParamsForPlaneBody();
	ComponentId CreatePlane(Body body, CM::Texture planeTexture, CM::AnimatedTexture thrustAnimatedTexture, PlaneParams params = PlaneParams());
	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 referenceVelocity,
		glm::vec2 initialVelocity, CM::Texture missileTexture, CM::AnimatedTexture thrustAnimatedTexture, std::optional<CM::Plane> planeId = std::nullopt,
		std::optional<CM::SoundBuffer> missileSoundBuffer = std::nullopt);
	void CreateExplosion(ExplosionParams params);
	void CreateSparking(SparkingParams params);
	void CreateFogForeground(int numOfLayers, float alphaPerLayer, CM::Texture fogTexture, FVec4 fColor = glm::vec4(1.0f), std::function<glm::vec2(int layer)> textureTranslation = nullptr);
	void CreateJuliaBackground(JuliaParams params);
	Components::Sound& CreateAndPlaySound(CM::SoundBuffer soundBuffer, FVec2 posF = nullptr,
		std::function<void(Components::Sound&)> config = nullptr, std::function<void(Components::Sound&)> stepF = nullptr);
	std::function<void(b2Fixture&, b2Fixture&)> SkipDuplicatedBodiesCollisions(std::function<void(b2Fixture&, b2Fixture&)> handler);
}
