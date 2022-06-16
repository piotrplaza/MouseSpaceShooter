#include "genericBuffers.hpp"

namespace Buffers
{
	GenericBuffers::GenericBuffers()
	{
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &positionBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);

		glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);
		glDisableVertexAttribArray(1);
	}

	GenericBuffers::GenericBuffers(GenericBuffers&& other) noexcept:
		vertexArray(other.vertexArray),
		positionBuffer(other.positionBuffer),
		colorBuffer(other.colorBuffer),
		texCoordBuffer(other.texCoordBuffer),
		modelMatrixF(std::move(other.modelMatrixF)),
		texture(other.texture),
		renderingSetup(other.renderingSetup),
		customShadersProgram(other.customShadersProgram),
		resolutionMode(other.resolutionMode),
		numOfPositions(other.numOfPositions),
		numOfColors(other.numOfColors),
		numOfTexCoord(other.numOfTexCoord),
		numOfAllocatedPositions(other.numOfAllocatedPositions),
		numOfAllocatedColors(other.numOfAllocatedColors),
		numOfAllocatedTexCoord(other.numOfAllocatedTexCoord),
		drawMode(other.drawMode),
		bufferDataUsage(other.bufferDataUsage),
		allocatedBufferDataUsage(std::move(other.allocatedBufferDataUsage)),
		preserveTextureRatio(other.preserveTextureRatio)
	{
		other.expired = true;
	}

	GenericBuffers::~GenericBuffers()
	{
		if (expired) return;

		glDeleteBuffers(1, &positionBuffer);
		if (colorBuffer) glDeleteBuffers(1, &*colorBuffer);
		if (texCoordBuffer) glDeleteBuffers(1, &*texCoordBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}

	void GenericBuffers::allocateOrUpdatePositionsBuffer(const std::vector<glm::vec3>& positions)
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

	void GenericBuffers::allocateOrUpdateColorsBuffer(const std::vector<glm::vec2>& colors)
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

	void GenericBuffers::allocateOrUpdateTexCoordBuffer(const std::vector<glm::vec2>& texCoord)
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

	void GenericBuffers::draw() const
	{
		glBindVertexArray(vertexArray);

		glDrawArrays(drawMode, 0, numOfPositions);
	}

	void GenericBuffers::createColorBuffer()
	{
		assert(!colorBuffer);
		colorBuffer = 0;
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &*colorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, *colorBuffer);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(1);
	}

	void GenericBuffers::createTexCoordBuffer()
	{
		assert(!texCoordBuffer);
		texCoordBuffer = 0;
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &*texCoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, *texCoordBuffer);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(2);
	}
}
