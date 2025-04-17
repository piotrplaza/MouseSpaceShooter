#include "Particles2D.hpp"

#include <components/graphicsSettings.hpp>

#include <components/camera2D.hpp>
#include <components/decoration.hpp>
#include <components/particles.hpp>
#include <components/mouse.hpp>
#include <components/physics.hpp>
#include <components/texture.hpp>
#include <components/mvp.hpp>
#include <components/systemInfo.hpp>
#include <globals/components.hpp>

#include <ogl/shaders/billboards.hpp>
#include <ogl/shaders/trails.hpp>

#include <tools/Shapes2D.hpp>
#include <tools/splines.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

namespace
{
	struct Params
	{
		const enum class RenderMode { Points, Lines, Billboards } renderMode;
		const enum class BlendMode { Alpha, Additive } blendMode;
		const bool manualControl;
		const float mouseSensitivity;
		const float duration;
		const unsigned controlPointsCount;
		const unsigned particlesCount;
		const unsigned instancesCount;
		const float maxColorComponent;
		const float velocityFactor;
		const glm::vec2 hSize;
		const bool forceRefreshRateBasedStep;
		const glm::vec3 globalForce;
		const glm::vec3 AZPlusBPlusCT;
		const std::optional<std::array<glm::vec4, 2>> forcedColors;
	};

	constexpr Params params1 = {
		.renderMode = Params::RenderMode::Billboards,
		.blendMode = Params::BlendMode::Additive,
		.manualControl = false,
		.mouseSensitivity = 0.002f,
		.duration = 120.0f,
		.controlPointsCount = 60,
		.particlesCount = 1000,
		.instancesCount = 3,
		.maxColorComponent = 0.005f,
		.velocityFactor = 10.0f,
		.hSize = glm::vec2(1.0f),
		.forceRefreshRateBasedStep = true,
		.globalForce = glm::vec3(0.0f, 0.0f, 0.0f),
		.AZPlusBPlusCT = glm::vec3(0.0f, 0.1f, 1.0f),
		.forcedColors = std::nullopt
	};

	constexpr Params params2 = {
		.renderMode = Params::RenderMode::Billboards,
		.blendMode = Params::BlendMode::Additive,
		.manualControl = false,
		.mouseSensitivity = 0.002f,
		.duration = 60.0f,
		.controlPointsCount = 240,
		.particlesCount = 10000,
		.instancesCount = 3,
		.maxColorComponent = 0.02f,
		.velocityFactor = 10.0f,
		.hSize = glm::vec2(1.0f),
		.forceRefreshRateBasedStep = false,
		.globalForce = glm::vec3(0.0f, -1.0f, 0.0f),
		.AZPlusBPlusCT = glm::vec3(0.0f, 0.01f, 0.2f),
		.forcedColors = std::nullopt
	};

	constexpr Params params3 = {
		.renderMode = Params::RenderMode::Billboards,
		.blendMode = Params::BlendMode::Additive,
		.manualControl = true,
		.mouseSensitivity = 0.002f,
		.duration = 60.0f,
		.controlPointsCount = 240,
		.particlesCount = 10000,
		.instancesCount = 1,
		.maxColorComponent = 0.02f,
		.velocityFactor = 4.0f,
		.hSize = glm::vec2(1.0f),
		.forceRefreshRateBasedStep = false,
		.globalForce = glm::vec3(0.0f, -1.0f, 0.0f),
		.AZPlusBPlusCT = glm::vec3(0.0f, 0.01f, 0.2f),
		.forcedColors = std::array<glm::vec4, 2>{ glm::vec4(0.005f, 0.005f, 0.005f, 1.0f), glm::vec4(0.005f, 0.05f, 0.005f, 1.0f) }
	};

	constexpr Params params4 = {
		.renderMode = Params::RenderMode::Points,
		.blendMode = Params::BlendMode::Additive,
		.manualControl = true,
		.mouseSensitivity = 0.002f,
		.duration = 60.0f,
		.controlPointsCount = 240,
		.particlesCount = 10000,
		.instancesCount = 1,
		.maxColorComponent = 0.02f,
		.velocityFactor = 4.0f,
		.hSize = glm::vec2(1.0f),
		.forceRefreshRateBasedStep = false,
		.globalForce = glm::vec3(0.0f, -1.0f, 0.0f),
		.AZPlusBPlusCT = glm::vec3(0.0f, 0.01f, 0.2f),
		.forcedColors = std::array<glm::vec4, 2>{ glm::vec4(1.0f, 1.0f, 0.3f, 1.0f), glm::vec4(1.0f, 0.5f, 0.3f, 1.0f) }
	};

	constexpr Params params5 = {
		.renderMode = Params::RenderMode::Lines,
		.blendMode = Params::BlendMode::Additive,
		.manualControl = true,
		.mouseSensitivity = 0.002f,
		.duration = 60.0f,
		.controlPointsCount = 240,
		.particlesCount = 10000,
		.instancesCount = 1,
		.maxColorComponent = 0.02f,
		.velocityFactor = 4.0f,
		.hSize = glm::vec2(1.0f),
		.forceRefreshRateBasedStep = false,
		.globalForce = glm::vec3(0.0f, -1.0f, 0.0f),
		.AZPlusBPlusCT = glm::vec3(0.0f, 0.01f, 0.2f),
		.forcedColors = std::array<glm::vec4, 2>{ glm::vec4(1.0f, 1.0f, 0.3f, 1.0f), glm::vec4(1.0f, 0.5f, 0.3f, 1.0f) }
	};

	constexpr Params params = params5;
}

namespace Levels
{
	class Particles2D::Impl
	{
	public:
		Particles2D::Impl()
		{
			auto& textures = Globals::Components().staticTextures();

			explosionTexture = textures.emplace(TextureFile("textures/rounded cloud.jpg", 0, true, TextureFile::AdditionalConversion::None, [](float* data, glm::ivec2 size, int numOfChannels) {
				for (int y = 0; y < size.y; ++y)
					for (int x = 0; x < size.x; ++x)
					{
						glm::vec3& pixel = reinterpret_cast<glm::vec3&>(*(data + (y * size.x + x) * numOfChannels));
						pixel = glm::vec3((pixel.x + pixel.y + pixel.z) / 3);
					}
			}));
		}

		void setup()
		{
			auto& graphicsSettings = Globals::Components().graphicsSettings();
			auto& physics = Globals::Components().physics();
			auto& camera = Globals::Components().camera2D();
			auto& particles = Globals::Components().particles();
			auto& decorations = Globals::Components().staticDecorations();

			graphicsSettings.pointSize = 1.0f;
			graphicsSettings.lineWidth = 1.0f;

			physics.forceRefreshRateBasedStep = params.forceRefreshRateBasedStep;

			camera.targetPositionAndProjectionHSizeF = glm::vec3(0.0f, 0.0f, camera.details.projectionHSize = camera.details.prevProjectionHSize = 1.0f);

			if (params.manualControl)
			{
				auto& cursor = decorations.emplace(Tools::Shapes2D::CreatePositionsOfCircle(glm::vec2(0.0f), 0.005f, 20));
				cursor.modelMatrixF = [&]() { return glm::translate(glm::mat4(1.0f), glm::vec3(cursorPosition[0], 0.0f)); };
			}
			else
			{
				for (auto& spline : splines)
				{
					spline = std::make_unique<const Tools::CubicHermiteSpline<>>([]() {
						std::vector<glm::vec2> points;
						points.reserve(params.controlPointsCount);
						for (unsigned i = 0; i < params.controlPointsCount; ++i)
							points.emplace_back(glm::linearRand(-params.hSize, params.hSize) * glm::vec2(Globals::Components().systemInfo().screen.getAspectRatio(), 1.0f));
						points.emplace_back(0.0f);
						return points;
					}());
				}
			}

			color[0] = { params.maxColorComponent, 0.0f, 0.0f };

			if constexpr (params.instancesCount > 1)
				color[1] = { 0.0f, params.maxColorComponent, 0.0f };

			if constexpr (params.instancesCount > 2)
				color[2] = { 0.0f, 0.0f, params.maxColorComponent };

			if constexpr (params.instancesCount > 3)
				for (unsigned i = 3; i < params.instancesCount; ++i)
					color[i] = glm::vec3(glm::linearRand(0.0f, params.maxColorComponent), glm::linearRand(0.0f, params.maxColorComponent), glm::linearRand(0.0f, params.maxColorComponent));

			createParticles();
		}

		void step()
		{
			const auto& camera = Globals::Components().camera2D();
			const auto& mouse = Globals::Components().mouse();
			const auto& physics = Globals::Components().physics();

			for (unsigned i = 0; i < params.instancesCount; ++i)
			{
				prevCursorPosition[i] = cursorPosition[i];
				if (params.manualControl)
				{
					cursorPosition[i] += mouse.getCartesianDelta() * params.mouseSensitivity;
					cursorPosition[i] = glm::clamp(cursorPosition[i], -camera.details.completeProjectionHSize, camera.details.completeProjectionHSize);
				}
				else
				{
					cursorPosition[i] = splines[i]->getSplineSample(std::min(1.0f, physics.simulationDuration / params.duration));
				}
			}

			cameraStep();
		}

	private:
		void cameraStep()
		{
			const auto& mouse = Globals::Components().mouse();
			auto& camera = Globals::Components().camera2D();

			cameraProjectionHSize -= mouse.pressed.wheel * 0.5f;

			camera.targetPositionAndProjectionHSizeF = glm::vec3(0.0f, 0.0f, cameraProjectionHSize);
		}

		void createParticles()
		{
			//const auto& camera = Globals::Components().camera2D();
			const auto& physics = Globals::Components().physics();
			const auto& mouse = Globals::Components().mouse();
			auto& particles = Globals::Components().particles();

			auto& billboardsShader = Globals::Shaders().billboards();
			auto& trailsShader = Globals::Shaders().trails();

			for (unsigned i = 0; i < params.instancesCount; ++i)
			{
				auto& particlesInstance = particles.emplace(
					std::make_pair([&, i]() { return glm::vec3(prevCursorPosition[i], 0.0f); }, [&, i]() { return glm::vec3(cursorPosition[i], 0.0f); }),
					[&, i, angle = 0.0f]() mutable {
						angle += 2.0f * physics.frameDuration * (mouse.pressing.lmb - mouse.pressing.rmb);
						return (params.manualControl ? glm::vec3(std::cos(angle), std::sin(angle), 0.0f) : glm::vec3(cursorPosition[i] - prevCursorPosition[i], 0.0f)) * params.velocityFactor; },
					glm::vec2(0.2f, 2.0f),
					params.forcedColors ? std::array<FVec4, 2>{(*params.forcedColors)[0], (*params.forcedColors)[1]} : std::array<FVec4, 2>{ glm::vec4(color[i], 1.0f), glm::vec4(color[i], 1.0f) },
					glm::vec2(0.2f, 1.0f),
					glm::pi<float>() * 0.05f,
					params.globalForce,
					true,
					params.particlesCount
				);

				particlesInstance.tfRenderingSetupF = [&, initRS = std::move(particlesInstance.tfRenderingSetupF)](auto& programBase) mutable {
					return [&, initRT = initRS(programBase), initRS = std::move(initRS)]() mutable {
						initRT();
						particlesInstance.tfRenderingSetupF = [&, initRS = std::move(initRS)](auto& programBase) mutable {
							particlesInstance.tfRenderingSetupF = [&, initRS = std::move(initRS)](auto& programBase) mutable {
								auto& tfParticles = static_cast<ShadersUtils::Programs::TFParticles&>(programBase);
								tfParticles.AZPlusBPlusCT(params.AZPlusBPlusCT);
								tfParticles.velocityFactor(0.0f);
								return initRS(programBase);
							};

							return nullptr;
						};
					};
				};

				if (params.renderMode == Params::RenderMode::Billboards)
				{
					particlesInstance.customShadersProgram = &billboardsShader;
					particlesInstance.renderingSetupF = [&](ShadersUtils::ProgramId program) mutable -> std::function<void()> {
						billboardsShader.vp(Globals::Components().mvp2D().getVP());
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, explosionTexture.component->loaded.textureObject);
						billboardsShader.texture0(0);

						if (params.blendMode == Params::BlendMode::Additive)
						{
							glBlendFunc(GL_SRC_ALPHA, GL_ONE);
							return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
						}
						
						return nullptr;
					};
				}
				else if (params.renderMode == Params::RenderMode::Points)
				{
					particlesInstance.renderingSetupF = [&](ShadersUtils::ProgramId program) mutable -> std::function<void()> {
						if (params.blendMode == Params::BlendMode::Additive)
						{
							glBlendFunc(GL_SRC_ALPHA, GL_ONE);
							return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
						}

						return nullptr;
					};
				}
				else if (params.renderMode == Params::RenderMode::Lines)
				{
					particlesInstance.customShadersProgram = &trailsShader;
					particlesInstance.renderingSetupF = [&](ShadersUtils::ProgramId program) mutable -> std::function<void()> {
						trailsShader.vp(Globals::Components().mvp2D().getVP());

						if (params.blendMode == Params::BlendMode::Additive)
						{
							glBlendFunc(GL_SRC_ALPHA, GL_ONE);
							return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
						}

						return nullptr;
					};
				}

				particlesInstance.resolutionMode = { ResolutionMode::Resolution::H1080, ResolutionMode::Scaling::Linear };
			}
		}

		std::array<glm::vec2, params.instancesCount> cursorPosition{};
		std::array<glm::vec2, params.instancesCount> prevCursorPosition{};
		std::array<glm::vec3, params.instancesCount> color{};
		std::array<ComponentId, params.instancesCount> particlesIds{};
		CM::Texture explosionTexture;

		float cameraProjectionHSize = 1.0f;

		std::array<std::unique_ptr<const Tools::CubicHermiteSpline<>>, params.instancesCount> splines;
	};

	Particles2D::Particles2D() :
		impl(std::make_unique<Impl>())
	{
	}

	void Particles2D::postSetup()
	{
		impl->setup();
	}

	Particles2D::~Particles2D() = default;

	void Particles2D::step()
	{
		impl->step();
	}
}
