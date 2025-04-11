#include "Particles2D2.hpp"

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

#include <tools/Shapes2D.hpp>
#include <tools/splines.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

namespace
{
	constexpr float mouseSensitivity = 0.002f;
	constexpr float duration = 60.0f;
	constexpr unsigned controlPointsCount = 240;
	constexpr unsigned particlesCount = 10000;
	constexpr unsigned instancesCount = 3;
	constexpr float maxColorComponent = 0.02f;
	constexpr float velocityFactor = 10.0f;
	constexpr glm::vec2 hSize = glm::vec2(1.0f);
	constexpr glm::vec2 initVelocityRange = glm::vec2(0.0f, 0.5f);
}

namespace Levels
{
	class Particles2D2::Impl
	{
	public:
		Particles2D2::Impl()
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
			auto& camera = Globals::Components().camera2D();
			auto& particles = Globals::Components().particles();
			auto& decorations = Globals::Components().staticDecorations();

			graphicsSettings.pointSize = 2.0f;
			graphicsSettings.lineWidth = 1.0f;

			camera.targetPositionAndProjectionHSizeF = glm::vec3(0.0f, 0.0f, camera.details.projectionHSize = camera.details.prevProjectionHSize = 1.0f);

			for (auto& spline : splines)
			{
				spline = std::make_unique<const Tools::CubicHermiteSpline<>>([]() {
					std::vector<glm::vec2> points;
					points.reserve(controlPointsCount);
					for (unsigned i = 0; i < controlPointsCount; ++i)
						points.emplace_back(glm::linearRand(-hSize, hSize) * glm::vec2(Globals::Components().systemInfo().screen.getAspectRatio(), 1.0f));
					return points;
					}());
			}

			color[0] = { maxColorComponent, 0.0f, 0.0f };

			if constexpr (instancesCount > 1)
				color[1] = { 0.0f, maxColorComponent, 0.0f };

			if constexpr (instancesCount > 2)
				color[2] = { 0.0f, 0.0f, maxColorComponent };

			if constexpr (instancesCount > 3)
				for (unsigned i = 3; i < instancesCount; ++i)
					color[i] = glm::vec3(glm::linearRand(0.0f, maxColorComponent), glm::linearRand(0.0f, maxColorComponent), glm::linearRand(0.0f, maxColorComponent));

			createParticles();
		}

		void step()
		{
			const auto& camera = Globals::Components().camera2D();
			const auto& mouse = Globals::Components().mouse();
			const auto& physics = Globals::Components().physics();

			for (unsigned i = 0; i < instancesCount; ++i)
			{
				prevCursorPosition[i] = cursorPosition[i];
				cursorPosition[i] = splines[i]->getSplineSample(std::min(1.0f, physics.simulationDuration / duration));
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
			const auto& camera = Globals::Components().camera2D();
			const auto& physics = Globals::Components().physics();
			const auto& mouse = Globals::Components().mouse();
			auto& particles = Globals::Components().particles();

			auto& billboardsShader = Globals::Shaders().billboards();

			for (unsigned i = 0; i < instancesCount; ++i)
			{
				auto& particlesId = particlesIds[i];

				if (particlesId)
					particles[particlesId].state = ComponentState::Outdated;

				auto& particlesInstance = particles.emplace(
					std::make_pair([&, i]() { return glm::vec3(prevCursorPosition[i], 0.0f); }, [&, i]() { return glm::vec3(cursorPosition[i], 0.0f); }),
					[&, i]() mutable { return glm::vec3(cursorPosition[i] - prevCursorPosition[i], 0.0f) * velocityFactor; },
					glm::vec2(0.2f, 2.0f),
					std::array<FVec4, 2>{ [&, i]() { return glm::vec4(color[i], 1.0f); }, glm::vec4(color[i], 1.0f) },
					glm::vec2(0.2f, 1.0f),
					glm::pi<float>() * 0.05f,
					glm::vec3(0.0f, -1.0f, 0.0f),
					true,
					particlesCount
				);

				particlesInstance.tfRenderingSetupF = [&, initRS = std::move(particlesInstance.tfRenderingSetupF)](auto& programBase) mutable {
					return [&, initRT = initRS(programBase), initRS = std::move(initRS)]() mutable {
						initRT();
						particlesInstance.tfRenderingSetupF = [&, initRS = std::move(initRS)](auto& programBase) mutable {
							particlesInstance.tfRenderingSetupF = [&, initRS = std::move(initRS)](auto& programBase) mutable {
								auto& tfParticles = static_cast<ShadersUtils::Programs::TFParticles&>(programBase);
								tfParticles.AZPlusBPlusCT({ 0.0f, 0.01f, 0.2f });
								tfParticles.velocityFactor(0.0f);
								return initRS(programBase);
							};

							return nullptr;
						};
					};
				};

				particlesInstance.customShadersProgram = &billboardsShader;

				particlesInstance.renderingSetupF = [&](ShadersUtils::ProgramId program) mutable {
					billboardsShader.vp(Globals::Components().mvp2D().getVP());
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, explosionTexture.component->loaded.textureObject);
					billboardsShader.texture0(0);

					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					billboardsShader.color(glm::vec4(1.0f));
					return std::function<void()>([]() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); });
				};

				particlesId = particlesInstance.getComponentId();
			}

			//billboards.emplace(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.2f, 0.0f), glm::vec2(-0.2f, 0.2f), 1000);
		}

		std::array<glm::vec2, instancesCount> cursorPosition{};
		std::array<glm::vec2, instancesCount> prevCursorPosition{};
		std::array<glm::vec3, instancesCount> color{};
		std::array<ComponentId, instancesCount> particlesIds{};
		CM::Texture explosionTexture;

		float cameraProjectionHSize = 1.0f;

		std::array<std::unique_ptr<const Tools::CubicHermiteSpline<>>, instancesCount> splines;
	};

	Particles2D2::Particles2D2() :
		impl(std::make_unique<Impl>())
	{
	}

	void Particles2D2::postSetup()
	{
		impl->setup();
	}

	Particles2D2::~Particles2D2() = default;

	void Particles2D2::step()
	{
		impl->step();
	}
}
