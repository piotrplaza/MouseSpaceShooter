#pragma once

#include <vector>
#include <functional>
#include <optional>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <ogl/shaders.hpp>

namespace Components
{
	struct Decoration
	{
		Decoration(std::vector<glm::vec3> positions, std::optional<unsigned> texture = std::nullopt,
			std::function<void(Shaders::ProgramId)> renderingSetup = nullptr):
			positions(std::move(positions)),
			texture(texture),
			renderingSetup(std::move(renderingSetup))
		{
		}

		const std::vector<glm::vec3>& getPositionsCache() const
		{
			return positions;
		}

		std::vector<glm::vec3> positions;
		std::optional<unsigned> texture;
		std::function<void(Shaders::ProgramId)> renderingSetup;
	};
}
