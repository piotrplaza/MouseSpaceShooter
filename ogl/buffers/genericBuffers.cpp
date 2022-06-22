#include "genericBuffers.hpp"

namespace Buffers
{
	SubGenericBuffers::SubGenericBuffers()
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

	SubGenericBuffers::SubGenericBuffers(SubGenericBuffers&& other) noexcept:
		vertexArray(other.vertexArray),
		positionBuffer(other.positionBuffer),
		colorBuffer(other.colorBuffer),
		texCoordBuffer(other.texCoordBuffer),
		modelMatrixF(std::move(other.modelMatrixF)),
		texture(other.texture),
		renderingSetup(other.renderingSetup),
		customShadersProgram(other.customShadersProgram),
		resolutionMode(other.resolutionMode),
		numOfVertices(other.numOfVertices),
		numOfColors(other.numOfColors),
		numOfTexCoord(other.numOfTexCoord),
		numOfAllocatedVertices(other.numOfAllocatedVertices),
		numOfAllocatedColors(other.numOfAllocatedColors),
		numOfAllocatedTexCoord(other.numOfAllocatedTexCoord),
		drawMode(other.drawMode),
		bufferDataUsage(other.bufferDataUsage),
		allocatedBufferDataUsage(std::move(other.allocatedBufferDataUsage)),
		preserveTextureRatio(other.preserveTextureRatio),
		render(other.render)
	{
		other.expired = true;
	}

	SubGenericBuffers::~SubGenericBuffers()
	{
		if (expired) return;

		glDeleteBuffers(1, &positionBuffer);
		if (colorBuffer) glDeleteBuffers(1, &*colorBuffer);
		if (texCoordBuffer) glDeleteBuffers(1, &*texCoordBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}

	void SubGenericBuffers::allocateOrUpdateVerticesBuffer(const std::vector<glm::vec3>& vertices)
	{
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		if (numOfAllocatedVertices < vertices.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices.front()), vertices.data(), bufferDataUsage);
			numOfAllocatedVertices = vertices.size();
			allocatedBufferDataUsage = bufferDataUsage;
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vertices.front()), vertices.data());
		}
		numOfVertices = vertices.size();
	}

	void SubGenericBuffers::allocateOrUpdateColorsBuffer(const std::vector<glm::vec4>& colors)
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

	void SubGenericBuffers::allocateOrUpdateTexCoordBuffer(const std::vector<glm::vec2>& texCoord)
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

	void SubGenericBuffers::createColorBuffer()
	{
		assert(!colorBuffer);
		colorBuffer = 0;
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &*colorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, *colorBuffer);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(1);
	}

	void SubGenericBuffers::createTexCoordBuffer()
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
