#pragma once

#include <ogl/shaders.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <optional>
#include <functional>
#include <vector>

namespace Buffers
{
	struct PosColorBuffers
	{
		PosColorBuffers();
		PosColorBuffers(const PosColorBuffers&) = delete;
		PosColorBuffers(PosColorBuffers&& other) noexcept;

		~PosColorBuffers();

		void allocateOrUpdatePositionsBuffer(const std::vector<glm::vec3>& positions);
		void allocateOrUpdateColorsBuffer(const std::vector<glm::vec4>& colors);

		void draw() const;

		GLenum drawMode = GL_LINES;
		GLenum bufferDataUsage = GL_DYNAMIC_DRAW;

	private:
		void createColorBuffer();

		GLuint vertexArray;
		GLuint positionBuffer;
		std::optional<GLuint> colorBuffer;
		size_t numOfPositions = 0;
		size_t numOfColors = 0;
		size_t numOfAllocatedPositions = 0;
		size_t numOfAllocatedColors = 0;
		std::optional<GLenum> allocatedBufferDataUsage;
		bool expired = false;
	};
}
