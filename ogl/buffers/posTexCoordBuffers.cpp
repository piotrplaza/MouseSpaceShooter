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
		modelMatrixF(std::move(other.modelMatrixF)),
		texture(other.texture),
		renderingSetup(other.renderingSetup),
		customShadersProgram(other.customShadersProgram),
		resolutionMode(other.resolutionMode),
		numOfPositions(other.numOfPositions),
		numOfTexCoord(other.numOfTexCoord),
		numOfAllocatedPositions(other.numOfAllocatedPositions),
		numOfAllocatedTexCoord(other.numOfAllocatedTexCoord),
		drawMode(other.drawMode),
		bufferDataUsage(other.bufferDataUsage),
		allocatedBufferDataUsage(std::move(other.allocatedBufferDataUsage)),
		preserveTextureRatio(other.preserveTextureRatio)
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

	void PosTexCoordBuffers::allocateOrUpdatePositionsBuffer(const std::vector<glm::vec3>& positions)
	{
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		if (numOfAllocatedPositions < positions.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(positions.front()), positions.data(), bufferDataUsage);
			numOfAllocatedPositions = positions.size();
			allocatedBufferDataUsage = bufferDataUsage;
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(positions.front()), positions.data());
		}
		numOfPositions = positions.size();
	}

	void PosTexCoordBuffers::allocateOrUpdateTexCoordBuffer(const std::vector<glm::vec2>& texCoord)
	{
		if (!texCoordBuffer)
			createTexCoordBuffer();

		glBindBuffer(GL_ARRAY_BUFFER, *texCoordBuffer);
		if (numOfAllocatedTexCoord < texCoord.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, texCoord.size() * sizeof(texCoord.front()), texCoord.data(), bufferDataUsage);
			numOfAllocatedTexCoord = texCoord.size();
			allocatedBufferDataUsage = bufferDataUsage;
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, texCoord.size() * sizeof(texCoord.front()), texCoord.data());
		}
		numOfTexCoord = texCoord.size();
	}

	void PosTexCoordBuffers::draw() const
	{
		glBindVertexArray(vertexArray);
		glDrawArrays(drawMode, 0, numOfPositions);
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
