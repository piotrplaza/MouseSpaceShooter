#pragma once

#include <ogl/shaders.hpp>

#include <commonTypes/resolutionMode.hpp>
#include <components/typeComponentMappers.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <optional>
#include <functional>
#include <vector>
#include <variant>

namespace Buffers
{
	struct PosTexCoordBuffers
	{
		PosTexCoordBuffers();
		PosTexCoordBuffers(const PosTexCoordBuffers&) = delete;
		PosTexCoordBuffers(PosTexCoordBuffers&& other) noexcept;

		~PosTexCoordBuffers();

		void createTexCoordBuffer();

		GLuint vertexArray;
		GLuint positionBuffer;

		std::optional<GLuint> texCoordBuffer;
		TextureComponentVariant texture;
		ComponentId renderingSetup = 0;
		std::optional<Shaders::ProgramId> customShadersProgram;
		ResolutionMode resolutionMode = ResolutionMode::Normal;

		std::vector<glm::vec3> positionsCache;
		std::vector<glm::vec2> texCoordCache;
		size_t numOfAllocatedPositions = 0;
		size_t numOfAllocatedTexCoord = 0;

		GLenum drawMode = GL_TRIANGLES;
		GLenum bufferDataUsage = GL_STATIC_DRAW;
		std::optional<GLenum> allocatedBufferDataUsage;

		bool preserveTextureRatio = false;

		bool expired = false;
	};
}
