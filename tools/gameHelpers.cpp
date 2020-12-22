#include "gameHelpers.hpp"

#include <globals.hpp>

#include <components/player.hpp>
#include <components/physics.hpp>
#include <components/decoration.hpp>

#include <ogl/uniformControllers.hpp>

#include <tools/animations.hpp>

namespace Tools
{
	PlayerPlaneHandler CreatePlayerPlane(unsigned planeTexture, unsigned flameAnimationTexture)
	{
		using namespace Globals::Components;

		PlayerPlaneHandler playerPlaneHandler;

		playerPlaneHandler.playerId = players.size();

		auto& player = players.emplace_back(Tools::CreateTrianglePlayerBody(2.0f, 0.2f), planeTexture);
		player.setPosition({ -10.0f, 0.0f });
		player.renderingSetup = [
			colorUniform = Uniforms::UniformController4f()
		](Shaders::ProgramId program) mutable {
			if (!colorUniform.isValid()) colorUniform = Uniforms::UniformController4f(program, "color");
			const float fade = (glm::sin(Globals::Components::physics.simulationTime * 2.0f * glm::two_pi<float>()) + 1.0f) / 2.0f;
			colorUniform.setValue({ fade, 1.0f, fade, 1.0f });

			return nullptr;
		};

		for (int i = 0; i < 2; ++i)
		{
			playerPlaneHandler.backThrustsBackgroundDecorationIds[i] = backgroundDecorations.size();
			auto& decoration = backgroundDecorations.emplace_back(Tools::CreatePositionsOfRectangle({ 0.0f, -0.45f }, { 0.5f, 0.5f }), flameAnimationTexture);

			decoration.renderingSetup = [&, i, modelUniform = Uniforms::UniformControllerMat4f(),
				thrustScale = 1.0f
			](Shaders::ProgramId program) mutable {
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
				modelUniform.setValue(glm::scale(glm::rotate(glm::translate(Tools::GetModelMatrix(*player.body),
					{ -0.9f, i == 0 ? -0.42f : 0.42f, 0.0f }),
					-glm::half_pi<float>() + (i == 0 ? 0.1f : -0.1f), { 0.0f, 0.0f, 1.0f }),
					{ std::min(thrustScale * 0.5f, 0.7f), thrustScale, 1.0f }));

				const float targetFrameTimeFactor = physics.frameTime * 6;
				if (player.throttling) thrustScale = std::min(thrustScale * (1.0f + targetFrameTimeFactor), 5.0f);
				else thrustScale = 1.0f + (thrustScale - 1.0f) * (1.0f - targetFrameTimeFactor);

				glBlendFunc(GL_ONE, GL_ONE);

				return []() { glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); };
			};

			decoration.animationController.reset(new Tools::TextureAnimationController(
				{ 500, 498 }, { 2, 0 }, { 61, 120 }, { 8, 4 }, { 62.5f, 124.9f }, 0.02f, 0,
				AnimationLayout::Horizontal, AnimationPlayback::Backward, AnimationPolicy::Repeat,
				{ 0.0f, -0.45f }, { 1.0f, 1.0f }));

			decoration.animationController->start();
		}

		return playerPlaneHandler;
	}
}
