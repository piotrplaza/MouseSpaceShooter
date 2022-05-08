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

		void allocateOrUpdatePositionsBuffer(const std::vector<glm::vec3>& positions);
		void allocateOrUpdateTexCoordBuffer(const std::vector<glm::vec2>& texCoord);

		void draw() const;

		TextureComponentVariant texture;
		ComponentId renderingSetup = 0;
		std::optional<Shaders::ProgramId> customShadersProgram;
		ResolutionMode resolutionMode = ResolutionMode::Normal;

		GLenum drawMode = GL_TRIANGLES;
		GLenum bufferDataUsage = GL_STATIC_DRAW;

		bool preserveTextureRatio = false;

	private:
		void createTexCoordBuffer();

		GLuint vertexArray;
		GLuint positionBuffer;
		std::optional<GLuint> texCoordBuffer;
		size_t numOfPositions = 0;
		size_t numOfTexCoord = 0;
		size_t numOfAllocatedPositions = 0;
		size_t numOfAllocatedTexCoord = 0;
		std::optional<GLenum> allocatedBufferDataUsage;
		bool expired = false;
	};
}
