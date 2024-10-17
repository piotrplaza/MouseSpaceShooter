#include "genericBuffers.hpp"

#include <ogl/oglProxy.hpp>
#include <tools/utility.hpp>

#include <algorithm>
#include <execution>

namespace
{
	constexpr bool parallelProcessing = true;

	template <typename BuffersContainer>
	inline typename BuffersContainer::value_type& ReuseOrEmplaceBack(BuffersContainer& buffersContainer, typename BuffersContainer::iterator& it)
	{
		return it == buffersContainer.end()
			? buffersContainer.emplace_back(), it = buffersContainer.end(), buffersContainer.back()
			: *it++;
	}

	inline void RenderableCommonsToBuffersCommons(RenderableDef& renderableDef, Buffers::GenericSubBuffers& buffers)
	{
		buffers.setVerticesBuffer(renderableDef.getVertices());
		buffers.setColorsBuffer(renderableDef.getColors());

		if (!std::holds_alternative<std::monostate>(renderableDef.texture))
			buffers.setTexCoordsBuffer(renderableDef.getTexCoords());

		if (renderableDef.params3D)
			buffers.setNormalsBuffer(renderableDef.params3D->normals_);

		buffers.setIndicesBuffer(renderableDef.getIndices());
	}
}

namespace Buffers
{
	GenericSubBuffers::GenericSubBuffers()
	{
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray_proxy(vertexArray);

		createPositionsBuffer();
		setColorsBuffer({});
		setInstancedTransformsBuffer({});
		setInstancedNormalTransformsBuffer({});
	}

	GenericSubBuffers::GenericSubBuffers(GenericSubBuffers&& other) noexcept:
		renderable(other.renderable),
		vertexArray(other.vertexArray),
		drawCount(other.drawCount),
		instanceCount(other.instanceCount),
		positionsBuffer(other.positionsBuffer),
		colorsBuffer(other.colorsBuffer),
		texCoordsBuffer(other.texCoordsBuffer),
		normalsBuffer(other.normalsBuffer),
		instancedTransformsBuffer(other.instancedTransformsBuffer),
		instancedNormalTransformsBuffer(other.instancedNormalTransformsBuffer),
		indicesBuffer(other.indicesBuffer),
		numOfAllocatedVertices(other.numOfAllocatedVertices),
		numOfAllocatedColors(other.numOfAllocatedColors),
		numOfAllocatedTexCoords(other.numOfAllocatedTexCoords),
		numOfAllocatedNormals(other.numOfAllocatedNormals),
		numOfAllocatedInstancedTransforms(other.numOfAllocatedInstancedTransforms),
		numOfAllocatedInstancedNormalTransforms(other.numOfAllocatedInstancedNormalTransforms),
		numOfAllocatedIndices(other.numOfAllocatedIndices),
		allocatedBufferDataUsage(other.allocatedBufferDataUsage)
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

		if (instancedTransformsBuffer)
			glDeleteBuffers(1, &*instancedTransformsBuffer);

		if (instancedNormalTransformsBuffer)
			glDeleteBuffers(1, &*instancedNormalTransformsBuffer);

		if (indicesBuffer)
			glDeleteBuffers(1, &*indicesBuffer);

		glDeleteVertexArrays(1, &vertexArray);
	}

	void GenericSubBuffers::setVerticesBuffer(const std::vector<glm::vec3>& vertices)
	{
		glBindBuffer(GL_ARRAY_BUFFER, positionsBuffer);
		if (numOfAllocatedVertices < vertices.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices.front()), vertices.data(), renderable->bufferDataUsage);
			numOfAllocatedVertices = vertices.size();
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vertices.front()), vertices.data());

		glEnableVertexAttribArray(0);

		drawCount = vertices.size();
	}

	void GenericSubBuffers::setColorsBuffer(const std::vector<glm::vec4>& colors)
	{
		glBindVertexArray_proxy(vertexArray);

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

		if (numOfAllocatedColors < colors.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(colors.front()), colors.data(), renderable->bufferDataUsage);
			numOfAllocatedColors = colors.size();
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size() * sizeof(colors.front()), colors.data());

		glEnableVertexAttribArray(1);
	}

	void GenericSubBuffers::setTexCoordsBuffer(const std::vector<glm::vec2>& texCoords)
	{
		glBindVertexArray_proxy(vertexArray);

		if (texCoords.empty())
		{
			glDisableVertexAttribArray(2);
			return;
		}

		if (texCoordsBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, *texCoordsBuffer);
		else
			createTexCoordsBuffer();

		if (numOfAllocatedTexCoords < texCoords.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(texCoords.front()), texCoords.data(), renderable->bufferDataUsage);
			numOfAllocatedTexCoords = texCoords.size();
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, texCoords.size() * sizeof(texCoords.front()), texCoords.data());

		glEnableVertexAttribArray(2);
	}

	void GenericSubBuffers::setNormalsBuffer(const std::vector<glm::vec3>& normals)
	{
		glBindVertexArray_proxy(vertexArray);

		if (normals.empty())
		{
			glDisableVertexAttribArray(3);
			return;
		}

		if (normalsBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, *normalsBuffer);
		else
			createNormalsBuffer();

		if (numOfAllocatedNormals < normals.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals.front()), normals.data(), renderable->bufferDataUsage);
			numOfAllocatedNormals = normals.size();
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, normals.size() * sizeof(normals.front()), normals.data());

		glEnableVertexAttribArray(3);
	}

	void GenericSubBuffers::setInstancedTransformsBuffer(const std::vector<glm::mat4>& transforms)
	{
		glBindVertexArray_proxy(vertexArray);

		if (transforms.empty())
		{
			static const glm::mat4 identity(1.0f);
			for (unsigned i = 0; i < 4; ++i)
			{
				glVertexAttrib4fv(4 + i, &identity[i][0]);
				glDisableVertexAttribArray(4 + i);
			}
			return;
		}

		if (instancedTransformsBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, *instancedTransformsBuffer);
		else
			createInstancedTransformsBuffer();

		if (numOfAllocatedInstancedTransforms < transforms.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(transforms.front()), transforms.data(), renderable->bufferDataUsage);
			numOfAllocatedInstancedTransforms = transforms.size();
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, transforms.size() * sizeof(transforms.front()), transforms.data());

		for (unsigned i = 0; i < 4; ++i)
			glEnableVertexAttribArray(4 + i);

		instanceCount = transforms.size();
	}

	void GenericSubBuffers::setInstancedNormalTransformsBuffer(const std::vector<glm::mat3>& transforms)
	{
		glBindVertexArray_proxy(vertexArray);

		if (transforms.empty())
		{
			static const glm::mat3 identity(1.0f);
			for (unsigned i = 0; i < 3; ++i)
			{
				glVertexAttrib4fv(8 + i, &identity[i][0]);
				glDisableVertexAttribArray(8 + i);
			}
			return;
		}

		if (instancedNormalTransformsBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, *instancedNormalTransformsBuffer);
		else
			createInstancedNormalTransformsBuffer();

		if (numOfAllocatedInstancedNormalTransforms < transforms.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(transforms.front()), transforms.data(), renderable->bufferDataUsage);
			numOfAllocatedInstancedNormalTransforms = transforms.size();
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, transforms.size() * sizeof(transforms.front()), transforms.data());

		for (unsigned i = 0; i < 3; ++i)
			glEnableVertexAttribArray(8 + i);
	}

	void GenericSubBuffers::setIndicesBuffer(const std::vector<unsigned>& indices)
	{
		glBindVertexArray_proxy(vertexArray);

		if (indices.empty())
			return;

		if (indicesBuffer)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indicesBuffer);
		else
			createIndicesBuffer();

		if (numOfAllocatedIndices < indices.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices.front()), indices.data(), renderable->bufferDataUsage);
			numOfAllocatedIndices = indices.size();
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}
		else
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(indices.front()), indices.data());

		drawCount = indices.size();
	}

	bool GenericSubBuffers::isInstancingActive() const
	{
		return numOfAllocatedInstancedTransforms;
	}

	bool GenericSubBuffers::isIndicingActive() const
	{
		return numOfAllocatedIndices;
	}

	void GenericSubBuffers::createPositionsBuffer()
	{
		glGenBuffers(1, &positionsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionsBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
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

	void GenericSubBuffers::createInstancedTransformsBuffer()
	{
		assert(!instancedTransformsBuffer);
		instancedTransformsBuffer = 0;
		glGenBuffers(1, &*instancedTransformsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, *instancedTransformsBuffer);
		for (unsigned i = 0; i < 4; ++i)
		{
			glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
			glVertexAttribDivisor(4 + i, 1);
		}
	}

	void GenericSubBuffers::createInstancedNormalTransformsBuffer()
	{
		assert(!instancedNormalTransformsBuffer);
		instancedNormalTransformsBuffer = 0;
		glGenBuffers(1, &*instancedNormalTransformsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, *instancedNormalTransformsBuffer);
		for (unsigned i = 0; i < 3; ++i)
		{
			glVertexAttribPointer(8 + i, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(sizeof(glm::vec3) * i));
			glVertexAttribDivisor(8 + i, 1);
		}
	}

	void GenericSubBuffers::createIndicesBuffer()
	{
		assert(!indicesBuffer);
		indicesBuffer = 0;
		glGenBuffers(1, &*indicesBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indicesBuffer);
	}

	GenericBuffers::GenericBuffers(GenericBuffers&& other) noexcept :
		GenericSubBuffers(std::move(other)),

		renderable(other.renderable),
		subsequence(std::move(other.subsequence)),
		normalTransforms(std::move(other.normalTransforms))
	{
	}

	const std::vector<glm::mat3>& GenericBuffers::calcNormalTransforms(const std::vector<glm::mat4>& transforms)
	{
		normalTransforms.resize(transforms.size());

		auto inverseTranspose = [&](const auto i) {
			normalTransforms[i] = glm::transpose(glm::inverse(glm::mat3(transforms[i])));
		};

		if constexpr (parallelProcessing && 1)
		{
			Tools::ItToId itToId(transforms.size());
			std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), inverseTranspose);
		}
		else
		{
			for (size_t i = 0; i < transforms.size(); ++i)
				inverseTranspose(i);
		}

		return normalTransforms;
	}

	void GenericBuffers::applyComponent(Renderable& renderableComponent, bool staticComponent)
	{
		auto applyMainPart = [&]() {
			GenericSubBuffers::renderable = &renderableComponent;
			renderable = &renderableComponent;

			if (!staticComponent && (!renderableComponent.isEnabled() || !renderableComponent.renderF()) && renderableComponent.loaded.buffers)
				return;

			RenderableCommonsToBuffersCommons(renderableComponent, *this);

			if (renderableComponent.instancing)
			{
				setInstancedTransformsBuffer(renderableComponent.instancing->transforms_);
				if (renderableComponent.params3D && !renderableComponent.params3D->gpuSideInstancedNormalTransforms_)
					setInstancedNormalTransformsBuffer(calcNormalTransforms(renderableComponent.instancing->transforms_));
			}
		};

		auto applySubsequencePart = [&]() {
			auto subBuffersIt = subsequence.begin();
			for (auto& renderableDef : renderableComponent.subsequence)
			{
				auto& subBuffers = ReuseOrEmplaceBack(subsequence, subBuffersIt);

				subBuffers.renderable = &renderableDef;
				renderableDef.loaded.subBuffers = &subBuffers;

				if (!staticComponent && (!renderableComponent.isEnabled() || !renderableComponent.renderF()) && renderableComponent.loaded.buffers)
					continue;

				RenderableCommonsToBuffersCommons(renderableDef, subBuffers);
			}
			subsequence.resize(std::distance(subsequence.begin(), subBuffersIt));
		};

		applyMainPart();
		applySubsequencePart();

		renderableComponent.loaded.buffers = this;
	}
}
