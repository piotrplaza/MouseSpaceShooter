#include "genericBuffers.hpp"

namespace
{
	template <typename BuffersContainer>
	inline typename BuffersContainer::value_type& ReuseOrEmplaceBack(BuffersContainer& buffersContainer, typename BuffersContainer::iterator& it)
	{
		return it == buffersContainer.end()
			? buffersContainer.emplace_back(), it = buffersContainer.end(), buffersContainer.back()
			: *it++;
	}

	inline void RenderableCommonsToBuffersCommons(RenderableDef& renderableDef, Buffers::GenericSubBuffers& buffers)
	{
		buffers.modelMatrixF = [&]() { return renderableDef.getModelMatrix(); };
		buffers.colorF = &renderableDef.colorF;
		buffers.renderingSetup = &renderableDef.renderingSetup;
		buffers.texture = &renderableDef.texture;
		buffers.drawMode = &renderableDef.drawMode;
		buffers.bufferDataUsage = &renderableDef.bufferDataUsage;
		buffers.preserveTextureRatio = &renderableDef.preserveTextureRatio;
		buffers.renderF = &renderableDef.renderF;

		buffers.setVerticesBuffer(renderableDef.getVertices());
		buffers.setColorsBuffer(renderableDef.getColors());
		buffers.setTexCoordBuffer(renderableDef.getTexCoord());
	}
}

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
		colorF(other.colorF),
		texture(other.texture),
		renderingSetup(other.renderingSetup),
		customShadersProgram(other.customShadersProgram),
		numOfVertices(other.numOfVertices),
		numOfAllocatedVertices(other.numOfAllocatedVertices),
		numOfAllocatedColors(other.numOfAllocatedColors),
		numOfAllocatedTexCoord(other.numOfAllocatedTexCoord),
		drawMode(other.drawMode),
		bufferDataUsage(other.bufferDataUsage),
		allocatedBufferDataUsage(std::move(other.allocatedBufferDataUsage)),
		preserveTextureRatio(other.preserveTextureRatio),
		renderF(other.renderF)
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

	void GenericSubBuffers::setVerticesBuffer(const std::vector<glm::vec3>& vertices)
	{
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		if (numOfAllocatedVertices < vertices.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != *bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices.front()), vertices.data(), *bufferDataUsage);
			numOfAllocatedVertices = vertices.size();
			allocatedBufferDataUsage = *bufferDataUsage;
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vertices.front()), vertices.data());
		}

		numOfVertices = vertices.size();
	}

	void GenericSubBuffers::setColorsBuffer(const std::vector<glm::vec4>& colors)
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
		
		if (numOfAllocatedColors < colors.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != *bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(colors.front()), colors.data(), *bufferDataUsage);
			numOfAllocatedColors = colors.size();
			allocatedBufferDataUsage = *bufferDataUsage;
		}
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size() * sizeof(colors.front()), colors.data());

		glEnableVertexAttribArray(1);
	}

	void GenericSubBuffers::setTexCoordBuffer(const std::vector<glm::vec2>& texCoord)
	{
		glBindVertexArray(vertexArray);

		if (texCoordBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, *texCoordBuffer);
		else
			createTexCoordBuffer();

		if (numOfAllocatedTexCoord < texCoord.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != *bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, texCoord.size() * sizeof(texCoord.front()), texCoord.data(), *bufferDataUsage);
			numOfAllocatedTexCoord = texCoord.size();
			allocatedBufferDataUsage = *bufferDataUsage;
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

	void GenericBuffers::applyComponentSubsequence(Renderable& renderableComponent)
	{
		auto subBuffersIt = subsequence.begin();
		for (auto& renderableDef : renderableComponent.subsequence)
		{
			auto& subBuffers = ReuseOrEmplaceBack(subsequence, subBuffersIt);
			RenderableCommonsToBuffersCommons(renderableDef, subBuffers);
			renderableDef.loaded.subBuffers = &subBuffers;
		}
		subsequence.resize(std::distance(subsequence.begin(), subBuffersIt));
	}

	void GenericBuffers::applyComponent(Renderable& renderableComponent)
	{
		customShadersProgram = &renderableComponent.customShadersProgram;
		resolutionMode = &renderableComponent.resolutionMode;
		subsequenceBegin = &renderableComponent.subsequenceBegin;
		posInSubsequence = &renderableComponent.posInSubsequence;

		RenderableCommonsToBuffersCommons(renderableComponent, *this);

		renderableComponent.loaded.buffers = this;
		renderableComponent.state = ComponentState::Ongoing;
	}
}
