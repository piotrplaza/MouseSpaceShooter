#include "posTexCoordBuffers.hpp"

namespace Buffers
{
	PosTexCoordBuffers::PosTexCoordBuffers()
	{
		glCreateVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &positionBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
	}

	PosTexCoordBuffers::PosTexCoordBuffers(PosTexCoordBuffers&& source) noexcept:
		vertexArray(source.vertexArray),
		positionBuffer(source.positionBuffer),
		texCoordBuffer(source.texCoordBuffer),
		texture(source.texture),
		renderingSetup(std::move(source.renderingSetup)),
		animationController(source.animationController),
		customShadersProgram(source.customShadersProgram),
		positionsCache(std::move(source.positionsCache)),
		texCoordCache(std::move(source.texCoordCache)),
		numOfAllocatedPositions(source.numOfAllocatedPositions),
		numOfAllocatedTexCoord(source.numOfAllocatedTexCoord)
	{
		source.expired = true;
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
