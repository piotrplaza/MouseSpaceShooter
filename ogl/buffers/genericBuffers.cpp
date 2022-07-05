#include "genericBuffers.hpp"

namespace Buffers
{
	GenericSubBuffers::GenericSubBuffers()
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

	GenericSubBuffers::GenericSubBuffers(GenericSubBuffers&& other) noexcept:
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

	GenericSubBuffers::~GenericSubBuffers()
	{
		if (expired)
			return;

		glDeleteBuffers(1, &positionBuffer);

		if (colorBuffer)
			glDeleteBuffers(1, &*colorBuffer);

		if (texCoordBuffer)
			glDeleteBuffers(1, &*texCoordBuffer);

		glDeleteVertexArrays(1, &vertexArray);
	}

	void GenericSubBuffers::allocateOrUpdateVerticesBuffer(const std::vector<glm::vec3>& vertices)
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

	void GenericSubBuffers::allocateOrUpdateColorsBuffer(const std::vector<glm::vec4>& colors)
	{
		glBindVertexArray(vertexArray);

		if (colors.empty())
		{
			glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);
			glDisableVertexAttribArray(1);
			return;
		}

		if (colorBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, *colorBuffer);
		else
			createColorBuffer();
		
		if (numOfAllocatedColors < colors.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(colors.front()), colors.data(), bufferDataUsage);
			numOfAllocatedColors = colors.size();
			allocatedBufferDataUsage = bufferDataUsage;
		}
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size() * sizeof(colors.front()), colors.data());

		glEnableVertexAttribArray(1);
	}

	void GenericSubBuffers::allocateOrUpdateTexCoordBuffer(const std::vector<glm::vec2>& texCoord)
	{
		glBindVertexArray(vertexArray);

		if (texCoordBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, *texCoordBuffer);
		else
			createTexCoordBuffer();

		if (numOfAllocatedTexCoord < texCoord.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, texCoord.size() * sizeof(texCoord.front()), texCoord.data(), bufferDataUsage);
			numOfAllocatedTexCoord = texCoord.size();
			allocatedBufferDataUsage = bufferDataUsage;
		}
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, texCoord.size() * sizeof(texCoord.front()), texCoord.data());

		glEnableVertexAttribArray(2);
	}

	void GenericSubBuffers::createColorBuffer()
	{
		assert(!colorBuffer);
		colorBuffer = 0;
		glGenBuffers(1, &*colorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, *colorBuffer);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	void GenericSubBuffers::createTexCoordBuffer()
	{
		assert(!texCoordBuffer);
		texCoordBuffer = 0;
		glGenBuffers(1, &*texCoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, *texCoordBuffer);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	}
}
