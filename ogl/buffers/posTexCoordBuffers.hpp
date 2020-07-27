#pragma once

#include <optional>
#include <functional>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <ogl/shaders.hpp>

#include <tools/animations.hpp>

namespace Buffers
{
	struct PosTexCoordBuffers
	{
		PosTexCoordBuffers();
		PosTexCoordBuffers(const PosTexCoordBuffers&) = delete;
		PosTexCoordBuffers(PosTexCoordBuffers&&) noexcept;

		~PosTexCoordBuffers();

		void createTexCoordBuffer();

		GLuint vertexArray;
		GLuint positionBuffer;

		std::optional<GLuint> texCoordBuffer;
		std::optional<unsigned> texture;
		std::function<std::function<void()>(Shaders::ProgramId)> renderingSetup;
		const Tools::TextureAnimationController* animationController = nullptr;
		std::optional<Shaders::ProgramId> customShadersProgram;

		std::vector<glm::vec3> positionsCache;
		std::vector<glm::vec2> texCoordCache;
		size_t numOfAllocatedPositions = 0;
		size_t numOfAllocatedTexCoord = 0;

		bool textureRatioPreserved = false;

		bool expired = false;
	};
}
