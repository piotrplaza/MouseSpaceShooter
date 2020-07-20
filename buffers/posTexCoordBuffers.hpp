#pragma once

#include <optional>
#include <functional>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <ogl/shaders.hpp>

namespace Buffers
{
	struct PosTexCoordBuffers
	{
		PosTexCoordBuffers();
		~PosTexCoordBuffers();

		void createTexCoordBuffer();

		GLuint vertexArray;
		GLuint positionBuffer;

		std::optional<GLuint> texCoordBuffer;
		std::optional<unsigned> texture;
		std::function<void(Shaders::ProgramId)> renderingSetup;
		std::optional<Shaders::ProgramId> customShadersProgram;

		std::vector<glm::vec3> positionsCache;
		std::vector<glm::vec2> texCoordCache;
		size_t numOfAllocatedPositions = 0;
		size_t numOfAllocatedTexCoord = 0;
	};
}
