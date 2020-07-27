#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <optional>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <ogl/shaders.hpp>

#include <tools/animations.hpp>

namespace Components
{
	struct Decoration
	{
		Decoration(std::vector<glm::vec3> positions = {}, std::optional<unsigned> texture = std::nullopt,
			std::function<std::function<void()>(Shaders::ProgramId)> renderingSetup = nullptr,
			std::unique_ptr<Tools::TextureAnimationController> animationController = nullptr,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			positions(std::move(positions)),
			texture(texture),
			renderingSetup(std::move(renderingSetup)),
			animationController(std::move(animationController)),
			customShadersProgram(customShadersProgram)
		{
		}

		const std::vector<glm::vec3>& getPositions() const
		{
			return positions;
		}

		const std::vector<glm::vec2> getTexCoord() const
		{
			const auto positions = getPositions();
			return std::vector<glm::vec2>(positions.begin(), positions.end());
		}

		std::vector<glm::vec3> positions;
		std::optional<unsigned> texture;
		std::function<std::function<void()>(Shaders::ProgramId)> renderingSetup;
		std::unique_ptr<Tools::TextureAnimationController> animationController;
		std::optional<Shaders::ProgramId> customShadersProgram;
	};
}
