#include "posColorBuffers.hpp"

namespace Buffers
{
	PosColorBuffers::PosColorBuffers()
	{
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &positionBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
	}

	PosColorBuffers::PosColorBuffers(PosColorBuffers&& other) noexcept :
		vertexArray(other.vertexArray),
		positionBuffer(other.positionBuffer),
		colorBuffer(other.colorBuffer),
		numOfPositions(other.numOfPositions),
		numOfColors(other.numOfColors),
		numOfAllocatedPositions(other.numOfAllocatedPositions),
		numOfAllocatedColors(other.numOfAllocatedColors),
		drawMode(other.drawMode),
		bufferDataUsage(other.bufferDataUsage),
		allocatedBufferDataUsage(std::move(other.allocatedBufferDataUsage))
	{
		other.expired = true;
	}

	PosColorBuffers::~PosColorBuffers()
	{
		if (expired) return;

		glDeleteBuffers(1, &positionBuffer);
		if (colorBuffer) glDeleteBuffers(1, &*colorBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}

	void PosColorBuffers::allocateOrUpdatePositionsBuffer(const std::vector<glm::vec3>& positions)
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

	void PosColorBuffers::allocateOrUpdateColorsBuffer(const std::vector<glm::vec4>& colors)
	{
		if (!colorBuffer)
			createColorBuffer();

		glBindBuffer(GL_ARRAY_BUFFER, *colorBuffer);
		if (numOfAllocatedColors < colors.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(colors.front()), colors.data(), bufferDataUsage);
			numOfAllocatedColors = colors.size();
			allocatedBufferDataUsage = bufferDataUsage;
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size() * sizeof(colors.front()), colors.data());
		}
		numOfColors = colors.size();
	}

	void PosColorBuffers::draw() const
	{
		glBindVertexArray(vertexArray);
		glDrawArrays(drawMode, 0, numOfPositions);
	}

	void PosColorBuffers::createColorBuffer()
	{
		assert(!colorBuffer);
		colorBuffer = 0;
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &*colorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, *colorBuffer);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(1);
	}
}
