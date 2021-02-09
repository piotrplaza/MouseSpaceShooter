#include "posTexCoordBuffers.hpp"

namespace Buffers
{
	PosTexCoordBuffers::PosTexCoordBuffers()
	{
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &positionBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
	}

	PosTexCoordBuffers::PosTexCoordBuffers(PosTexCoordBuffers&& other) noexcept:
		vertexArray(other.vertexArray),
		positionBuffer(other.positionBuffer),
		texCoordBuffer(other.texCoordBuffer),
		texture(other.texture),
		renderingSetup(std::move(other.renderingSetup)),
		animationController(other.animationController),
		customShadersProgram(other.customShadersProgram),
		positionsCache(std::move(other.positionsCache)),
		texCoordCache(std::move(other.texCoordCache)),
		numOfAllocatedPositions(other.numOfAllocatedPositions),
		numOfAllocatedTexCoord(other.numOfAllocatedTexCoord),
		drawMode(other.drawMode),
		bufferDataUsage(other.bufferDataUsage),
		allocatedBufferDataUsage(std::move(other.allocatedBufferDataUsage))
	{
		other.expired = true;
	}

	PosTexCoordBuffers::~PosTexCoordBuffers()
	{
		if (expired) return;

		glDeleteBuffers(1, &positionBuffer);
		if (texCoordBuffer) glDeleteBuffers(1, &*texCoordBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}

	void PosTexCoordBuffers::createTexCoordBuffer()
	{
		assert(!texCoordBuffer);
		texCoordBuffer = 0;
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &*texCoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, *texCoordBuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(1);
	}
}
