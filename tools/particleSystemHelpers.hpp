#pragma once

#include <commonTypes/componentMappers.hpp>
#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/fTypes.hpp>
#include <commonTypes/renderLayer.hpp>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <functional>
#include <optional>
#include <array>

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

	struct ParticleSystemParams
	{
		enum class RenderMode { Points, Lines, Billboards };
		enum class BlendMode { Alpha, Additive };

		ParticleSystemParams& position(FVec3 value)
		{
			position_ = std::move(value);
			return *this;
		}

		ParticleSystemParams& prevPosition(FVec3 value)
		{
			prevPosition_ = std::move(value);
			return *this;
		}

		ParticleSystemParams& resolutionMode(ResolutionMode value)
		{
			resolutionMode_ = value;
			return *this;
		}

		ParticleSystemParams& renderMode(RenderMode value)
		{
			renderMode_ = value;
			return *this;
		}

		ParticleSystemParams& blendMode(BlendMode value)
		{
			blendMode_ = value;
			return *this;
		}

		ParticleSystemParams& pointSize(float value)
		{
			pointSize_ = value;
			return *this;
		}

		ParticleSystemParams& lineWidth(float value)
		{
			lineWidth_ = value;
			return *this;
		}

		ParticleSystemParams& pointSmooth(bool value)
		{
			pointSmooth_ = value;
			return *this;
		}

		ParticleSystemParams& lineSmooth(bool value)
		{
			lineSmooth_ = value;
			return *this;
		}

		ParticleSystemParams& duration(float value)
		{
			duration_ = value;
			return *this;
		}

		ParticleSystemParams& particlesCount(unsigned value)
		{
			particlesCount_ = value;
			return *this;
		}

		ParticleSystemParams& velocityOffset(FVec3 value)
		{
			velocityOffset_ = std::move(value);
			return *this;
		}

		ParticleSystemParams& initVelocity(FVec3 value)
		{
			initVelocity_ = std::move(value);
			return *this;
		}

		ParticleSystemParams& velocityFactor(float value)
		{
			velocityFactor_ = value;
			return *this;
		}

		ParticleSystemParams& velocityRotateZHRange(float value)
		{
			velocityRotateZHRange_ = value;
			return *this;
		}

		ParticleSystemParams& velocitySpreadFactorRange(glm::vec2 value)
		{
			velocitySpreadFactorRange_ = value;
			return *this;
		}

		ParticleSystemParams& lifetimeRange(glm::vec2 value)
		{
			lifetimeRange_ = value;
			return *this;
		}

		ParticleSystemParams& respawning(bool value)
		{
			respawning_ = value;
			return *this;
		}

		ParticleSystemParams& globalForce(glm::vec3 value)
		{
			globalForce_ = value;
			return *this;
		}

		ParticleSystemParams& AZPlusBPlusCT(glm::vec3 value)
		{
			AZPlusBPlusCT_ = value;
			return *this;
		}

		ParticleSystemParams& originForce(float value)
		{
			originForce_ = value;
			return *this;
		}

		ParticleSystemParams& deltaTimeFactor(float value)
		{
			deltaTimeFactor_ = value;
			return *this;
		}

		ParticleSystemParams& colorsRange(std::array<FVec4, 2> value)
		{
			colorsRange_ = std::move(value);
			return *this;
		}

		ParticleSystemParams& firstInitVelocity(glm::vec2 value)
		{
			firstInitVelocity_ = value;
			return *this;
		}

		ParticleSystemParams& texture(CM::Texture value)
		{
			texture_ = value;
			return *this;
		}

		FVec3 position_ = glm::vec3(0.0f);
		FVec3 prevPosition_;
		ResolutionMode resolutionMode_ = { ResolutionMode::Resolution::HalfNative, ResolutionMode::Scaling::Linear };
		RenderMode renderMode_ = RenderMode::Lines;
		BlendMode blendMode_ = BlendMode::Additive;
		float pointSize_ = 2.0f;
		float lineWidth_ = 1.0f;
		float pointSmooth_ = true;
		float lineSmooth_ = false;
		float duration_ = 10.0f;
		unsigned particlesCount_ = 10000;
		FVec3 velocityOffset_ = glm::vec3(0.0f, 0.0f, 0.0f);
		FVec3 initVelocity_ = glm::vec3(4.0f, 0.0f, 0.0f);
		float velocityFactor_ = 1.0f;
		float velocityRotateZHRange_ = 0.05f;
		glm::vec2 velocitySpreadFactorRange_ = glm::vec2(0.2f, 1.0f);
		glm::vec2 lifetimeRange_ = glm::vec2(0.2f, 2.0f);
		bool respawning_ = true;
		glm::vec3 globalForce_ = glm::vec3(0.0f, -1.0f, 0.0f);
		glm::vec3 AZPlusBPlusCT_ = glm::vec3(0.0f, 0.01f, 0.2f);
		float originForce_ = 0.0f;
		float deltaTimeFactor_ = 2.0f;
		std::array<FVec4, 2> colorsRange_ = { glm::vec4(1.0f, 1.0f, 0.3f, 1.0f), glm::vec4(1.0f, 0.5f, 0.3f, 1.0f) };
		std::optional<glm::vec2> firstInitVelocity_ = std::nullopt;
		CM::Texture texture_;
	};

	void CreateExplosion(ExplosionParams params);
	void CreateSparking(SparkingParams params);
	void CreateParticleSystem(ParticleSystemParams params);

}
