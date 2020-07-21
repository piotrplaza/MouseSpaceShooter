#include "playground.hpp"

#include "globals.hpp"

#include "components/screenInfo.hpp"
#include "components/physics.hpp"
#include "components/textureDef.hpp"
#include "components/player.hpp"
#include "components/wall.hpp"
#include "components/grapple.hpp"
#include "components/camera.hpp"
#include "components/decoration.hpp"
#include "components/graphicsSettings.hpp"

#include "ogl/uniformControllers.hpp"

#include "shaders/julia.hpp"

namespace Levels
{

	struct Playground::Impl
	{
		Shaders::Programs::Julia juliaShaders;

		unsigned rocketPlaneTexture = 0;
		unsigned spaceRockTexture = 0;
		unsigned woodTexture = 0;
		unsigned orbTexture = 0;

		Components::Player* player1 = nullptr;
	};

	Playground::Playground(): impl(std::make_unique<Impl>())
	{
		setGraphicsSettings();
		setTextures();
		setPlayers();
		setBackground();
		setStaticWalls();
		setDynamicWalls();
		setGrapples();
		setCamera();
	}

	Playground::~Playground() = default;

	void Playground::setGraphicsSettings() const
	{
		using namespace Globals::Components;

		graphicsSettings.basicLevelColor = { 0.7f, 0.7f, 0.7f, 1.0f };
		graphicsSettings.texturedLevelColor = { 0.7f, 0.7f, 0.7f, 1.0f };
	}

	void Playground::setTextures() const
	{
		using namespace Globals::Components;

		impl->rocketPlaneTexture = texturesDef.size();
		texturesDef.emplace_back("textures/rocket plane.png");
		texturesDef.back().translate = glm::vec2(-0.35f, -0.5f);
		texturesDef.back().scale = glm::vec2(1.7f);

		impl->spaceRockTexture = texturesDef.size();
		texturesDef.emplace_back("textures/space rock.jpg", GL_MIRRORED_REPEAT);
		texturesDef.back().scale = glm::vec2(20.0f);

		impl->woodTexture = texturesDef.size();
		texturesDef.emplace_back("textures/wood.jpg", GL_MIRRORED_REPEAT);
		texturesDef.back().scale = glm::vec2(16.0f);

		impl->orbTexture = texturesDef.size();
		texturesDef.emplace_back("textures/orb.png");
		texturesDef.back().translate = glm::vec2(-0.5f);
		texturesDef.back().scale = glm::vec2(4.0f);
	}

	void Playground::setPlayers() const
	{
		using namespace Globals::Components;

		impl->player1 = &players.emplace_back(Tools::CreateTrianglePlayerBody(2.0f, 0.2f), impl->rocketPlaneTexture);
		impl->player1->setPosition({ -10.0f, 0.0f });
		impl->player1->renderingSetup = [
			colorUniform = Uniforms::UniformController4f()
		](Shaders::ProgramId program) mutable {
			if (!colorUniform.isValid()) colorUniform = Uniforms::GetUniformController4f(program, "color");
			const float fade = (glm::sin(Globals::Components::physics.simulationTime * 2.0f * glm::two_pi<float>()) + 1.0f) / 2.0f;
			colorUniform.setValue({ fade, 1.0f, fade, 1.0f });
		};

		foregroundDecorations.emplace_back(Tools::CreateRectanglePositions({ 0.0f, 0.0f }, { 0.5f, 0.5f }));
		foregroundDecorations.back().renderingSetup = [&,
			modelUniform = Uniforms::UniformControllerMat4f(),
			colorUniform = Uniforms::UniformController4f()
		](Shaders::ProgramId program) mutable {
			if (!modelUniform.isValid()) modelUniform = Uniforms::GetUniformControllerMat4f(program, "model");
			if (!colorUniform.isValid()) colorUniform = Uniforms::GetUniformController4f(program, "color");
			const float fade = (glm::sin(Globals::Components::physics.simulationTime * 3.0f * glm::two_pi<float>()) + 1.0f) / 2.0f;
			colorUniform.setValue({ 1.0f, fade, 0, 0.1f });
			modelUniform.setValue(glm::translate(Tools::GetModelMatrix(*impl->player1->body), { -1.5f, 0.0f, 0.0f }));
		};
	}

	void Playground::setBackground() const
	{
		using namespace Globals::Components;

		auto& background = backgroundDecorations.emplace_back(Tools::CreateRectanglePositions({ 0.0f, 0.0f }, { 10.0f, 10.0f }));
		background.customShadersProgram = impl->juliaShaders.program;
		background.renderingSetup = [&,
			vpUniform = Uniforms::UniformControllerMat4f(),
			juliaCOffsetUniform = Uniforms::UniformController2f(),
			minColorUniform = Uniforms::UniformController4f(),
			maxColorUniform = Uniforms::UniformController4f()
		](Shaders::ProgramId program) mutable {
			if (!vpUniform.isValid()) vpUniform = Uniforms::GetUniformControllerMat4f(program, "vp");
			if (!juliaCOffsetUniform.isValid()) juliaCOffsetUniform = Uniforms::GetUniformController2f(program, "juliaCOffset");
			if (!minColorUniform.isValid()) minColorUniform = Uniforms::GetUniformController4f(program, "minColor");
			if (!maxColorUniform.isValid()) maxColorUniform = Uniforms::GetUniformController4f(program, "maxColor");
			vpUniform.setValue(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-camera.prevPosition * 0.005f, 0.0f)),
				glm::vec3((float)screenInfo.windowSize.y / screenInfo.windowSize.x, 1.0f, 1.0f) * 1.5f));
			juliaCOffsetUniform.setValue(impl->player1->getPosition() * 0.00001f);
			minColorUniform.setValue({ 0.0f, 0.0f, 0.0f, 1.0f });
			maxColorUniform.setValue({ 0, 0.1f, 0.2f, 1.0f });
		};
	}

	void Playground::setStaticWalls() const
	{
		using namespace Globals::Components;

		const float levelHSize = 50.0f;
		const float bordersHGauge = 50.0f;
		staticWalls.emplace_back(Tools::CreateBoxBody({ -levelHSize - bordersHGauge, 0.0f },
			{ bordersHGauge, levelHSize + bordersHGauge * 2 }), impl->spaceRockTexture);
		staticWalls.emplace_back(Tools::CreateBoxBody({ levelHSize + bordersHGauge, 0.0f },
			{ bordersHGauge, levelHSize + bordersHGauge * 2 }), impl->spaceRockTexture);
		staticWalls.emplace_back(Tools::CreateBoxBody({ 0.0f, -levelHSize - bordersHGauge },
			{ levelHSize + bordersHGauge * 2, bordersHGauge }), impl->spaceRockTexture);
		staticWalls.emplace_back(Tools::CreateBoxBody({ 0.0f, levelHSize + bordersHGauge },
			{ levelHSize + bordersHGauge * 2, bordersHGauge }), impl->spaceRockTexture);
		staticWalls.emplace_back(Tools::CreateCircleBody({ 10.0f, 0.0f }, 2.0f), impl->spaceRockTexture);
		staticWalls.back().renderingSetup = [
			colorUniform = Uniforms::UniformController4f()
		](Shaders::ProgramId program) mutable {
				if (!colorUniform.isValid()) colorUniform = Uniforms::GetUniformController4f(program, "color");
				colorUniform.setValue({ 1.0f, 1.0f, 1.0f,
					(glm::sin(Globals::Components::physics.simulationTime * glm::two_pi<float>()) + 1.0f) / 2.0f });
			};
	}

	void Playground::setDynamicWalls() const
	{
		using namespace Globals::Components;

		auto& wall1 = *dynamicWalls.emplace_back(Tools::CreateBoxBody({ 5.0f, -5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f),
			impl->woodTexture).body;
		auto& wall2 = *dynamicWalls.emplace_back(Tools::CreateBoxBody({ 5.0f, 5.0f }, { 0.5f, 5.0f }, 0.0f, b2_dynamicBody, 0.2f),
			impl->woodTexture).body;
		wall1.GetFixtureList()->SetRestitution(0.5f);
		wall2.GetFixtureList()->SetRestitution(0.5f);
		Tools::PinBodies(wall1, wall2, { 5.0f, 0.0f });
		dynamicWalls.back().renderingSetup = [
			colorUniform = Uniforms::UniformController4f()
		](Shaders::ProgramId program) mutable {
				if (!colorUniform.isValid()) colorUniform = Uniforms::GetUniformController4f(program, "color");
				colorUniform.setValue({ 1.0f, 1.0f, 1.0f,
					(glm::sin(Globals::Components::physics.simulationTime / 2.0f * glm::two_pi<float>()) + 1.0f) / 2.0f });
			};
	}

	void Playground::setGrapples() const
	{
		using namespace Globals::Components;

		grapples.emplace_back(Tools::CreateCircleBody({ 0.0f, 10.0f }, 1.0f), 15.0f, impl->orbTexture);
		grapples.emplace_back(Tools::CreateCircleBody({ 0.0f, -10.0f }, 1.0f), 15.0f, impl->orbTexture);
		grapples.back().renderingSetup = [
			colorUniform = Uniforms::UniformController4f()
		](Shaders::ProgramId program) mutable {
				if (!colorUniform.isValid()) colorUniform = Uniforms::GetUniformController4f(program, "color");
				colorUniform.setValue({ 1.0f, 1.0f, 1.0f,
					(glm::sin(Globals::Components::physics.simulationTime / 3.0f * glm::two_pi<float>()) + 1.0f) / 2.0f });
			};
			grapples.emplace_back(Tools::CreateCircleBody({ -10.0f, -30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f), 30.0f,
				impl->orbTexture);
			grapples.emplace_back(Tools::CreateCircleBody({ -10.0f, 30.0f }, 2.0f, b2_dynamicBody, 0.1f, 0.2f), 30.0f);
	}

	void Playground::setCamera() const
	{
		using namespace Globals::Components;

		camera.targetProjectionHSizeF = []() {
			camera.projectionTransitionFactor = 0.1f * physics.targetFrameTimeFactor;
			return 15.0f + glm::length(players.front().getVelocity()) * 0.2f;
		};
		camera.targetPositionF = []() {
			camera.positionTransitionFactor = 0.1f * physics.targetFrameTimeFactor;
			return players.front().getPosition() + players.front().getVelocity() * 0.3f;
		};
	}
}
