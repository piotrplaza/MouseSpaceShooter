#include "particleSystemHelpers.hpp"

#include <components/physics.hpp>
#include <components/decoration.hpp>
#include <components/texture.hpp>
#include <components/vp.hpp>
#include <components/shockwave.hpp>
#include <components/particles.hpp>
#include <components/graphicsSettings.hpp>
#include <components/deferredAction.hpp>
#include <components/animatedTexture.hpp>
#include <components/blendingTexture.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <ogl/shaders/billboards.hpp>
#include <ogl/shaders/trails.hpp>
#include <ogl/shaders/tfParticles.hpp>

namespace Tools
{
	void CreateExplosion(ExplosionParams params)
	{
		auto& billboards = Globals::Shaders().billboards();

		auto explosionF = [&, params]() {
			auto& shockwave = Globals::Components().shockwaves().emplace(params.center_, params.sourceVelocity_, params.numOfParticles_, params.initExplosionVelocity_,
				params.initExplosionVelocityRandomMinFactor_, params.particlesRadius_, params.particlesDensity_, params.particlesLinearDamping_, params.particlesAsBullets_, params.particlesAsSensors_);
			auto& explosionDecoration = Globals::Components().decorations().emplace();
			explosionDecoration.customShadersProgram = &billboards;
			explosionDecoration.renderLayer = params.renderLayer_;
			explosionDecoration.targetTexture = Globals::Components().standardRenderTexture(params.renderMode_);
			explosionDecoration.drawMode = GL_POINTS;
			explosionDecoration.bufferDataUsage = GL_DYNAMIC_DRAW;

			explosionDecoration.renderingSetupF = [params, startTime = Globals::Components().physics().simulationDuration, &billboards](ShadersUtils::ProgramId program) mutable {
				billboards.vp(Globals::Components().vpDefault2D().getVP());
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, params.explosionTexture_.component->loaded.textureObject);
				billboards.texture0(0);

				const float elapsed = Globals::Components().physics().simulationDuration - startTime;

				if (params.additiveBlending_)
				{
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					billboards.color(glm::vec4(glm::vec3(glm::pow(1.0f - elapsed / (params.explosionDuration_ * 2.0f), 10.0f)), 1.0f) * params.color_);
					return std::function<void()>([]() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); });
				}
				else
				{
					billboards.color(glm::pow(1.0f - elapsed / (params.explosionDuration_ * 2.0f), 10.0f) * params.color_);
					return std::function<void()>();
				}
			};

			shockwave.stepF = [params, startTime = Globals::Components().physics().simulationDuration, &shockwave, &explosionDecoration]() {
				const float elapsed = Globals::Components().physics().simulationDuration - startTime;
				const float scale = params.presentationInitScale_ + elapsed * params.presentationScaleFactor_;

				if (elapsed > params.explosionDuration_)
				{
					shockwave.state = ComponentState::Outdated;

					if (params.endCallback_)
						params.endCallback_(shockwave);
				}

				explosionDecoration.positions.clear();
				for (size_t i = 0; i < shockwave.particles.size(); ++i)
				{
					if (i % params.particlesPerDecoration_ != 0)
						continue;
					const auto& particle = shockwave.particles[i];
					const glm::vec2 position = shockwave.center + (ToVec2<glm::vec2>(particle->GetWorldCenter()) - shockwave.center) * 0.5f;
					explosionDecoration.positions.emplace_back(position, scale);
				}
				explosionDecoration.state = ComponentState::Changed;
			};

			shockwave.teardownF = [&, prevTeardownF = std::move(shockwave.teardownF)]() {
				if (prevTeardownF)
					prevTeardownF();
				explosionDecoration.state = ComponentState::Outdated;
			};

			if (params.beginCallback_)
				params.beginCallback_(shockwave);
			};

		if (params.deferredExectution_)
			Globals::Components().deferredActions().emplace([explosionF = std::move(explosionF)](float) { explosionF(); return false; });
		else
			explosionF();
	}

	void CreateSparking(SparkingParams params)
	{
		auto& sparking = Globals::Components().particles().emplace([=]() { return glm::vec3(params.sourcePoint_, 0.0f); }, glm::vec3(0.0f), glm::vec3(params.initVelocity_, 0.0f), glm::vec2(0.0f, 2.0f),
			std::array<FVec4, 2>{ glm::vec4(1.0f, 1.0f, 0.3f, 1.0f), glm::vec4(1.0f, 0.5f, 0.3f, 1.0f) }, glm::vec2(params.initVelocityRandomMinFactor_, 1.0f), glm::pi<float>() * params.spreadFactor_,
			glm::vec3(params.gravity_, 0.0f), false, params.sparksCount_);
		auto& trails = Globals::Shaders().trails();
		sparking.customShadersProgram = &trails;
		sparking.renderingSetupF = [&, params](auto&) {
			Globals::Shaders().trails().vp(Globals::Components().vpDefault2D().getVP());
			Globals::Shaders().trails().deltaTimeFactor(params.trailsScale_);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glLineWidth(params.lineWidth_);
			return [&]() {
				glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				glLineWidth(Globals::Components().graphicsSettings().lineWidth);
			};
		};
	}

	void CreateParticleSystem(ParticleSystemParams params)
	{
		const auto& physics = Globals::Components().physics();
		const auto& mouse = Globals::Components().mouse();
		auto& particles = Globals::Components().particles();

		auto& billboardsShader = Globals::Shaders().billboards();
		auto& trailsShader = Globals::Shaders().trails();

		auto& particlesInstance = particles.emplace(
			params.prevPosition_.isLoaded() ? std::make_pair(params.position_, params.prevPosition_) : std::make_pair(params.position_, params.position_), params.velocityOffset_,
			[params]() mutable {
				if (params.firstInitVelocity_)
				{
					const auto result = glm::vec3(*params.firstInitVelocity_, 0.0f);
					params.firstInitVelocity_ = std::nullopt;
					return result;
				}
				return params.initVelocity_();
			},
			params.lifetimeRange_,
			params.colorsRange_,
			params.velocitySpreadFactorRange_,
			glm::pi<float>() * params.velocityRotateZHRange_,
			params.globalForce_,
			params.respawning_,
			params.particlesCount_
		);

		particlesInstance.tfRenderingSetupF = [&, params, initRS = std::move(particlesInstance.tfRenderingSetupF)](auto& programBase) mutable {
			auto& tfParticles = static_cast<ShadersUtils::Programs::TFParticles&>(programBase);
			auto initRT = initRS(programBase);
			tfParticles.originInit(true);
			return [&, params, initRT = std::move(initRT)]() mutable {
				initRT();
				particlesInstance.tfRenderingSetupF = [&, params, initRS = std::move(particlesInstance.tfRenderingSetupF)](auto&) mutable {
					auto initRT = initRS(tfParticles);
					tfParticles.AZPlusBPlusCT(params.AZPlusBPlusCT_);
					tfParticles.originForce(params.originForce_);
					tfParticles.velocityFactor(params.velocityFactor_);
					return initRT;
				};
			};
		};

		if (params.renderMode_ == ParticleSystemParams::DrawMode::Billboards)
		{
			particlesInstance.customShadersProgram = &billboardsShader;
			particlesInstance.renderingSetupF = [&, params](auto&) mutable -> std::function<void()> {
				billboardsShader.vp(Globals::Components().vpDefault2D().getVP());
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, params.texture_.component->loaded.textureObject);
				billboardsShader.texture0(0);

				if (params.blendMode_ == ParticleSystemParams::BlendMode::Additive)
				{
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
				}

				return nullptr;
			};
		}
		else if (params.renderMode_ == ParticleSystemParams::DrawMode::Points)
		{
			particlesInstance.renderingSetupF = [&, params](auto&) mutable -> std::function<void()> {
				const bool prevPointSmooth = glProxyIsPointSmoothEnabled();
				const float prevPointSize = glProxyGetPointSize();
				
				glProxySetPointSmooth(params.pointSmooth_);
				glPointSize(params.pointSize_);
				
				if (params.blendMode_ == ParticleSystemParams::BlendMode::Additive)
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				return [=]() {
					glProxySetPointSmooth(prevPointSmooth);
					glPointSize(prevPointSize);
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				};
			};
		}
		else if (params.renderMode_ == ParticleSystemParams::DrawMode::Lines)
		{
			particlesInstance.customShadersProgram = &trailsShader;
			particlesInstance.renderingSetupF = [&, params](auto&) mutable -> std::function<void()> {
				const bool prevLineSmooth = glProxyIsLineSmoothEnabled();
				const float prevLineWidth = glProxyGetLineWidth();

				glProxySetLineSmooth(params.lineSmooth_);
				glLineWidth(params.lineWidth_);

				trailsShader.vp(Globals::Components().vpDefault2D().getVP());
				trailsShader.deltaTimeFactor(params.deltaTimeFactor_);

				if (params.blendMode_ == ParticleSystemParams::BlendMode::Additive)
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				return [=]() {
					glProxySetLineSmooth(prevLineSmooth);
					glLineWidth(prevLineWidth);
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				};
			};
		}

		particlesInstance.targetTexture = Globals::Components().standardRenderTexture(params.resolutionMode_);
	}
}
