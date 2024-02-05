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
		buffers.originF = [&]() { return renderableDef.getOrigin(); };
		buffers.colorF = &renderableDef.colorF;
		buffers.renderingSetup = &renderableDef.renderingSetup;
		buffers.texture = &renderableDef.texture;
		buffers.drawMode = &renderableDef.drawMode;
		buffers.bufferDataUsage = &renderableDef.bufferDataUsage;
		buffers.preserveTextureRatio = &renderableDef.preserveTextureRatio;
		buffers.renderF = &renderableDef.renderF;

		buffers.setVerticesBuffer(renderableDef.getVertices());
		buffers.setColorsBuffer(renderableDef.getColors());

		if (!std::holds_alternative<std::monostate>(renderableDef.texture))
		{
			buffers.setTexCoordsBuffer(renderableDef.getTexCoords());
		}

		if (renderableDef.params3D)
		{
			buffers.setNormalsBuffer(renderableDef.params3D->normals_);
			buffers.ambient = &renderableDef.params3D->ambient_;
			buffers.diffuse = &renderableDef.params3D->diffuse_;
			buffers.specular = &renderableDef.params3D->specular_;
			buffers.specularFocus = &renderableDef.params3D->specularFocus_;
			buffers.lightModelEnabled = &renderableDef.params3D->lightModelEnabled_;
		}

		buffers.setIndicesBuffer(renderableDef.getIndices());
	}
}

namespace Buffers
{
	GenericSubBuffers::GenericSubBuffers()
	{
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &positionsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionsBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);

		glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);
		glDisableVertexAttribArray(1);
	}

	GenericSubBuffers::GenericSubBuffers(GenericSubBuffers&& other) noexcept :
		vertexArray(other.vertexArray),
		positionsBuffer(other.positionsBuffer),
		colorsBuffer(other.colorsBuffer),
		texCoordsBuffer(other.texCoordsBuffer),
		normalsBuffer(other.normalsBuffer),
		indicesBuffer(other.indicesBuffer),
		modelMatrixF(std::move(other.modelMatrixF)),
		originF(std::move(other.originF)),
		colorF(other.colorF),
		texture(other.texture),
		renderingSetup(other.renderingSetup),
		customShadersProgram(other.customShadersProgram),
		drawCount(other.drawCount),
		numOfAllocatedVertices(other.numOfAllocatedVertices),
		numOfAllocatedColors(other.numOfAllocatedColors),
		numOfAllocatedTexCoords(other.numOfAllocatedTexCoords),
		numOfAllocatedNormals(other.numOfAllocatedNormals),
		numOfAllocatedIndices(other.numOfAllocatedIndices),
		drawMode(other.drawMode),
		bufferDataUsage(other.bufferDataUsage),
		allocatedBufferDataUsage(std::move(other.allocatedBufferDataUsage)),
		preserveTextureRatio(other.preserveTextureRatio),
		renderF(other.renderF),
		ambient(other.ambient),
		diffuse(other.diffuse),
		specular(other.specular),
		specularFocus(other.specularFocus),
		lightModelEnabled(other.lightModelEnabled)
	{
		other.expired = true;
	}

	GenericSubBuffers::~GenericSubBuffers()
	{
		if (expired)
			return;

		glDeleteBuffers(1, &positionsBuffer);

		if (colorsBuffer)
			glDeleteBuffers(1, &*colorsBuffer);

		if (texCoordsBuffer)
			glDeleteBuffers(1, &*texCoordsBuffer);

		if (normalsBuffer)
			glDeleteBuffers(1, &*normalsBuffer);

		if (indicesBuffer)
			glDeleteBuffers(1, &*indicesBuffer);

		glDeleteVertexArrays(1, &vertexArray);
	}

	void GenericSubBuffers::setVerticesBuffer(const std::vector<glm::vec3>& vertices)
	{
		glBindBuffer(GL_ARRAY_BUFFER, positionsBuffer);
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

		drawCount = vertices.size();
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

		if (colorsBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, *colorsBuffer);
		else
			createColorsBuffer();

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

	void GenericSubBuffers::setTexCoordsBuffer(const std::vector<glm::vec2>& texCoords)
	{
		glBindVertexArray(vertexArray);

		if (texCoords.empty())
		{
			glDisableVertexAttribArray(2);
			return;
		}

		if (texCoordsBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, *texCoordsBuffer);
		else
			createTexCoordsBuffer();

		if (numOfAllocatedTexCoords < texCoords.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != *bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(texCoords.front()), texCoords.data(), *bufferDataUsage);
			numOfAllocatedTexCoords = texCoords.size();
			allocatedBufferDataUsage = *bufferDataUsage;
		}
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, texCoords.size() * sizeof(texCoords.front()), texCoords.data());

		glEnableVertexAttribArray(2);
	}

	void GenericSubBuffers::setNormalsBuffer(const std::vector<glm::vec3>& normals)
	{
		glBindVertexArray(vertexArray);

		if (normals.empty())
		{
			glDisableVertexAttribArray(3);
			return;
		}

		if (normalsBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, *normalsBuffer);
		else
			createNormalsBuffer();

		if (numOfAllocatedNormals < normals.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != *bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals.front()), normals.data(), *bufferDataUsage);
			numOfAllocatedNormals = normals.size();
			allocatedBufferDataUsage = *bufferDataUsage;
		}
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, normals.size() * sizeof(normals.front()), normals.data());

		glEnableVertexAttribArray(3);
	}

	void GenericSubBuffers::setIndicesBuffer(const std::vector<unsigned>& indices)
	{
		glBindVertexArray(vertexArray);

		if (indices.empty())
			return;

		if (indicesBuffer)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indicesBuffer);
		else
			createIndicesBuffer();

		if (numOfAllocatedIndices < indices.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != *bufferDataUsage)
		{
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices.front()), indices.data(), *bufferDataUsage);
			numOfAllocatedIndices = indices.size();
			allocatedBufferDataUsage = *bufferDataUsage;
		}
		else
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(indices.front()), indices.data());

		drawCount = indices.size();
	}

	bool GenericSubBuffers::isIndicesBufferActive() const
	{
		return numOfAllocatedIndices;
	}

	void GenericSubBuffers::createColorsBuffer()
	{
		assert(!colorsBuffer);
		colorsBuffer = 0;
		glGenBuffers(1, &*colorsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, *colorsBuffer);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	void GenericSubBuffers::createTexCoordsBuffer()
	{
		assert(!texCoordsBuffer);
		texCoordsBuffer = 0;
		glGenBuffers(1, &*texCoordsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, *texCoordsBuffer);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	void GenericSubBuffers::createNormalsBuffer()
	{
		assert(!normalsBuffer);
		normalsBuffer = 0;
		glGenBuffers(1, &*normalsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, *normalsBuffer);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	void GenericSubBuffers::createIndicesBuffer()
	{
		assert(!indicesBuffer);
		indicesBuffer = 0;
		glGenBuffers(1, &*indicesBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indicesBuffer);
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
