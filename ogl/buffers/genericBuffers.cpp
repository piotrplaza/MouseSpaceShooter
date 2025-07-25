#include "genericBuffers.hpp"

#include <ogl/oglProxy.hpp>
#include <tools/utility.hpp>

#include <algorithm>
#include <execution>

namespace
{
	constexpr bool parallelProcessing = true;
}

namespace Buffers
{
	GenericSubBuffers::GenericSubBuffers(bool defaultVAO)
	{
		if (!defaultVAO)
			glGenVertexArrays(1, &vertexArray);
		glProxyBindVertexArray(vertexArray);

		createPositionsBuffer();

		setColorsBuffer({});
		setVelocitiesAndTimesBuffer({});
		setHSizesAndAnglesBuffer({});
		setTexCoordsBuffer({});
		setNormalsBuffer({});
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
		velocitiesAndTimesBuffer(other.velocitiesAndTimesBuffer),
		hSizesAndAnglesBuffer(other.hSizesAndAnglesBuffer),
		texCoordsBuffer(other.texCoordsBuffer),
		normalsBuffer(other.normalsBuffer),
		instancedTransformsBuffer(other.instancedTransformsBuffer),
		instancedNormalTransformsBuffer(other.instancedNormalTransformsBuffer),
		indicesBuffer(other.indicesBuffer),

		numOfAllocatedPositions(other.numOfAllocatedPositions),
		numOfAllocatedColors(other.numOfAllocatedColors),
		numOfAllocatedVelocitiesAndTimes(other.numOfAllocatedVelocitiesAndTimes),
		numOfAllocatedHSizesAndAngles(other.numOfAllocatedHSizesAndAngles),
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

		if (velocitiesAndTimesBuffer)
			glDeleteBuffers(1, &*velocitiesAndTimesBuffer);

		if (hSizesAndAnglesBuffer)
			glDeleteBuffers(1, &*hSizesAndAnglesBuffer);

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

	void GenericSubBuffers::setPositionsBuffer(const std::vector<glm::vec3>& positions)
	{
		glProxyBindVertexArray(vertexArray);

		glBindBuffer(GL_ARRAY_BUFFER, positionsBuffer);
		if (numOfAllocatedPositions < positions.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(positions.front()), positions.data(), renderable->bufferDataUsage);
			numOfAllocatedPositions = positions.size();
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}
		else
			glBufferSubData(GL_ARRAY_BUFFER, positionAttribIdx, positions.size() * sizeof(positions.front()), positions.data());

		glEnableVertexAttribArray(positionAttribIdx);

		drawCount = (GLsizei)positions.size();
	}

	void GenericSubBuffers::setPositionsBuffer(glm::vec3 position, unsigned count)
	{
		glProxyBindVertexArray(vertexArray);

		glBindBuffer(GL_ARRAY_BUFFER, positionsBuffer);
		if (numOfAllocatedPositions < count || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, count * sizeof(position), nullptr, renderable->bufferDataUsage);
			numOfAllocatedPositions = count;
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}

		glClearBufferData(GL_ARRAY_BUFFER, GL_RGB32F, GL_RGB, GL_FLOAT, &position);
		glEnableVertexAttribArray(positionAttribIdx);

		drawCount = count;
	}

	void GenericSubBuffers::allocateTFPositionsBuffer(unsigned count)
	{
		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, positionsBuffer);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, positionAttribIdx, positionsBuffer);
		if (numOfAllocatedPositions < count || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, count * sizeof(glm::vec3), nullptr, renderable->bufferDataUsage);
			numOfAllocatedPositions = count;
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}

		drawCount = count;
	}

	void GenericSubBuffers::setColorsBuffer(const std::vector<glm::vec4>& colors)
	{
		glProxyBindVertexArray(vertexArray);

		if (colors.empty())
		{
			glVertexAttrib4f(colorAttribIdx, 1.0f, 1.0f, 1.0f, 1.0f);
			glDisableVertexAttribArray(colorAttribIdx);
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

		glEnableVertexAttribArray(colorAttribIdx);
	}

	void GenericSubBuffers::setColorsBuffer(glm::vec4 color, unsigned count)
	{
		glProxyBindVertexArray(vertexArray);

		if (count == 0)
		{
			glVertexAttrib4f(colorAttribIdx, color.x, color.y, color.z, color.w);
			glDisableVertexAttribArray(colorAttribIdx);
			return;
		}

		if (colorsBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, *colorsBuffer);
		else
			createColorsBuffer();

		if (numOfAllocatedColors < count || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, count * sizeof(color), nullptr, renderable->bufferDataUsage);
			numOfAllocatedColors = count;
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}

		glClearBufferData(GL_ARRAY_BUFFER, GL_RGBA32F, GL_RGBA, GL_FLOAT, &color);
		glEnableVertexAttribArray(colorAttribIdx);
	}

	void GenericSubBuffers::allocateTFColorsBuffer(unsigned count)
	{
		if (colorsBuffer)
			glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, *colorsBuffer);
		else
			createColorsBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, colorAttribIdx, *colorsBuffer);

		if (numOfAllocatedColors < count || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, count * sizeof(glm::vec4), nullptr, renderable->bufferDataUsage);
			numOfAllocatedColors = count;
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}
	}

	void GenericSubBuffers::setVelocitiesAndTimesBuffer(const std::vector<glm::vec4>& velocitiesAndTimes)
	{
		glProxyBindVertexArray(vertexArray);

		if (velocitiesAndTimes.empty())
		{
			glVertexAttrib4f(velocityAndTimeAttribIdx, 0.0f, 0.0f, 0.0f, 0.0f);
			glDisableVertexAttribArray(velocityAndTimeAttribIdx);
			return;
		}

		if (velocitiesAndTimesBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, *velocitiesAndTimesBuffer);
		else
			createVelocitiesAndTimesBuffer();

		if (numOfAllocatedVelocitiesAndTimes < velocitiesAndTimes.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, velocitiesAndTimes.size() * sizeof(velocitiesAndTimes.front()), velocitiesAndTimes.data(), renderable->bufferDataUsage);
			numOfAllocatedVelocitiesAndTimes = velocitiesAndTimes.size();
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, velocitiesAndTimes.size() * sizeof(velocitiesAndTimes.front()), velocitiesAndTimes.data());

		glEnableVertexAttribArray(velocityAndTimeAttribIdx);
	}

	void GenericSubBuffers::setVelocitiesAndTimesBuffer(glm::vec4 velocityAndTime, unsigned count)
	{
		glProxyBindVertexArray(vertexArray);

		if (count == 0)
		{
			glVertexAttrib4f(velocityAndTimeAttribIdx, velocityAndTime.x, velocityAndTime.y, velocityAndTime.z, velocityAndTime.w);
			glDisableVertexAttribArray(velocityAndTimeAttribIdx);
			return;
		}

		if (velocitiesAndTimesBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, *velocitiesAndTimesBuffer);
		else
			createVelocitiesAndTimesBuffer();

		if (numOfAllocatedVelocitiesAndTimes < count || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, count * sizeof(velocityAndTime), nullptr, renderable->bufferDataUsage);
			numOfAllocatedVelocitiesAndTimes = count;
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}

		glClearBufferData(GL_ARRAY_BUFFER, GL_RGBA32F, GL_RGBA, GL_FLOAT, &velocityAndTime);
		glEnableVertexAttribArray(velocityAndTimeAttribIdx);
	}

	void GenericSubBuffers::allocateTFVelocitiesAndTimesBuffer(unsigned count)
	{
		if (velocitiesAndTimesBuffer)
			glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, *velocitiesAndTimesBuffer);
		else
			createVelocitiesAndTimesBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, velocityAndTimeAttribIdx, *velocitiesAndTimesBuffer);

		if (numOfAllocatedVelocitiesAndTimes < count || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, count * sizeof(glm::vec4), nullptr, renderable->bufferDataUsage);
			numOfAllocatedVelocitiesAndTimes = count;
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}
	}

	void GenericSubBuffers::setHSizesAndAnglesBuffer(const std::vector<glm::vec3>& hSizes)
	{
		glProxyBindVertexArray(vertexArray);

		if (hSizes.empty())
		{
			glVertexAttrib3f(hSizeAndAngleAttribIdx, 0.5f, 0.5f, 0.0f);
			glDisableVertexAttribArray(hSizeAndAngleAttribIdx);
			return;
		}
		if (hSizesAndAnglesBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, *hSizesAndAnglesBuffer);
		else
			createHSizesAndAnglesBuffer();

		if (numOfAllocatedHSizesAndAngles < hSizes.size() || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, hSizes.size() * sizeof(hSizes.front()), hSizes.data(), renderable->bufferDataUsage);
			numOfAllocatedHSizesAndAngles = hSizes.size();
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, hSizes.size() * sizeof(hSizes.front()), hSizes.data());

		glEnableVertexAttribArray(hSizeAndAngleAttribIdx);
	}

	void GenericSubBuffers::setHSizesAndAnglesBuffer(glm::vec3 hSizeAndAngle, unsigned count)
	{
		glProxyBindVertexArray(vertexArray);

		if (count == 0)
		{
			glVertexAttrib3f(hSizeAndAngleAttribIdx, hSizeAndAngle.x, hSizeAndAngle.y, hSizeAndAngle.z);
			glDisableVertexAttribArray(hSizeAndAngleAttribIdx);
			return;
		}

		if (hSizesAndAnglesBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, *hSizesAndAnglesBuffer);
		else
			createHSizesAndAnglesBuffer();

		if (numOfAllocatedHSizesAndAngles < count || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_ARRAY_BUFFER, count * sizeof(hSizeAndAngle), nullptr, renderable->bufferDataUsage);
			numOfAllocatedHSizesAndAngles = count;
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}

		glClearBufferData(GL_ARRAY_BUFFER, GL_RGB32F, GL_RGB, GL_FLOAT, &hSizeAndAngle);
		glEnableVertexAttribArray(hSizeAndAngleAttribIdx);
	}

	void GenericSubBuffers::allocateTFHSizesAndAnglesBuffer(unsigned count)
	{
		if (hSizesAndAnglesBuffer)
			glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, *hSizesAndAnglesBuffer);
		else
			createHSizesAndAnglesBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, hSizeAndAngleAttribIdx, *hSizesAndAnglesBuffer);

		if (numOfAllocatedHSizesAndAngles < count || !allocatedBufferDataUsage || *allocatedBufferDataUsage != renderable->bufferDataUsage)
		{
			glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, count * sizeof(glm::vec3), nullptr, renderable->bufferDataUsage);
			numOfAllocatedHSizesAndAngles = count;
			allocatedBufferDataUsage = renderable->bufferDataUsage;
		}
	}

	void GenericSubBuffers::setTexCoordsBuffer(const std::vector<glm::vec2>& texCoords)
	{
		glProxyBindVertexArray(vertexArray);

		if (texCoords.empty())
		{
			glVertexAttrib2f(texCoordAttribIdx, 0.0f, 0.0f);
			glDisableVertexAttribArray(texCoordAttribIdx);
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

		glEnableVertexAttribArray(texCoordAttribIdx);
	}

	void GenericSubBuffers::setNormalsBuffer(const std::vector<glm::vec3>& normals)
	{
		glProxyBindVertexArray(vertexArray);

		if (normals.empty())
		{
			glVertexAttrib3f(normalAttribIdx, 0.0f, 0.0f, 1.0f);
			glDisableVertexAttribArray(normalAttribIdx);
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

		glEnableVertexAttribArray(normalAttribIdx);
	}

	void GenericSubBuffers::setInstancedTransformsBuffer(const std::vector<glm::mat4>& transforms)
	{
		glProxyBindVertexArray(vertexArray);

		if (transforms.empty())
		{
			static const glm::mat4 identity(1.0f);
			for (unsigned i = 0; i < 4; ++i)
			{
				glVertexAttrib4fv(instancedTransformAttribIdx + i, &identity[i][0]);
				glDisableVertexAttribArray(instancedTransformAttribIdx + i);
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
			glEnableVertexAttribArray(instancedTransformAttribIdx + i);

		instanceCount = (GLsizei)transforms.size();
	}

	void GenericSubBuffers::setInstancedNormalTransformsBuffer(const std::vector<glm::mat3>& transforms)
	{
		glProxyBindVertexArray(vertexArray);

		if (transforms.empty())
		{
			static const glm::mat3 identity(1.0f);
			for (unsigned i = 0; i < 3; ++i)
			{
				glVertexAttrib4fv(instancedNormalTransformAttribIdx + i, &identity[i][0]);
				glDisableVertexAttribArray(instancedNormalTransformAttribIdx + i);
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
			glEnableVertexAttribArray(instancedNormalTransformAttribIdx + i);
	}

	void GenericSubBuffers::setIndicesBuffer(const std::vector<unsigned>& indices)
	{
		glProxyBindVertexArray(vertexArray);

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

		drawCount = (GLsizei)indices.size();
	}

	bool GenericSubBuffers::isInstancingActive() const
	{
		return numOfAllocatedInstancedTransforms;
	}

	bool GenericSubBuffers::isIndicingActive() const
	{
		return numOfAllocatedIndices;
	}

	void GenericSubBuffers::swapActiveBuffers(GenericSubBuffers& other)
	{
		assert(other.vertexArray == 0);
		assert(renderable == other.renderable);
		assert(drawCount == other.drawCount);
		assert(instanceCount == other.instanceCount);
		assert(allocatedBufferDataUsage == other.allocatedBufferDataUsage);

		glProxyBindVertexArray(vertexArray);

		assert(numOfAllocatedPositions == other.numOfAllocatedPositions);
		std::swap(positionsBuffer, other.positionsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionsBuffer);
		glVertexAttribPointer(positionAttribIdx, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

		if (other.colorsBuffer && colorsBuffer)
		{
			assert(numOfAllocatedColors == other.numOfAllocatedColors);
			std::swap(colorsBuffer, other.colorsBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, *colorsBuffer);
			glVertexAttribPointer(colorAttribIdx, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		}

		if (other.velocitiesAndTimesBuffer && velocitiesAndTimesBuffer)
		{
			assert(numOfAllocatedVelocitiesAndTimes == other.numOfAllocatedVelocitiesAndTimes);
			std::swap(velocitiesAndTimesBuffer, other.velocitiesAndTimesBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, *velocitiesAndTimesBuffer);
			glVertexAttribPointer(velocityAndTimeAttribIdx, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		}

		if (other.hSizesAndAnglesBuffer && hSizesAndAnglesBuffer)
		{
			assert(numOfAllocatedHSizesAndAngles == other.numOfAllocatedHSizesAndAngles);
			std::swap(hSizesAndAnglesBuffer, other.hSizesAndAnglesBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, *hSizesAndAnglesBuffer);
			glVertexAttribPointer(hSizeAndAngleAttribIdx, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		}

		if (other.texCoordsBuffer && texCoordsBuffer)
		{
			assert(numOfAllocatedTexCoords == other.numOfAllocatedTexCoords);
			std::swap(texCoordsBuffer, other.texCoordsBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, *texCoordsBuffer);
			glVertexAttribPointer(texCoordAttribIdx, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		}

		if (other.normalsBuffer && normalsBuffer)
		{
			assert(numOfAllocatedNormals == other.numOfAllocatedNormals);
			std::swap(normalsBuffer, other.normalsBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, *normalsBuffer);
			glVertexAttribPointer(normalAttribIdx, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		}

		if (other.instancedTransformsBuffer && instancedTransformsBuffer)
		{
			assert(numOfAllocatedInstancedTransforms == other.numOfAllocatedInstancedTransforms);
			std::swap(instancedTransformsBuffer, other.instancedTransformsBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, *instancedTransformsBuffer);
			for (unsigned i = 0; i < 4; ++i)
			{
				glVertexAttribPointer(instancedTransformAttribIdx + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
				glVertexAttribDivisor(instancedTransformAttribIdx + i, 1);
			}
		}

		if (other.instancedNormalTransformsBuffer && instancedNormalTransformsBuffer)
		{
			assert(numOfAllocatedInstancedNormalTransforms == other.numOfAllocatedInstancedNormalTransforms);
			std::swap(instancedNormalTransformsBuffer, other.instancedNormalTransformsBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, *instancedNormalTransformsBuffer);
			for (unsigned i = 0; i < 3; ++i)
			{
				glVertexAttribPointer(instancedNormalTransformAttribIdx + i, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(sizeof(glm::vec3) * i));
				glVertexAttribDivisor(instancedNormalTransformAttribIdx + i, 1);
			}
		}

		if (other.indicesBuffer && indicesBuffer)
		{
			assert(numOfAllocatedIndices == other.numOfAllocatedIndices);
			std::swap(indicesBuffer, other.indicesBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indicesBuffer);
		}
	}

	void GenericSubBuffers::createPositionsBuffer(unsigned target)
	{
		glGenBuffers(1, &positionsBuffer);
		glBindBuffer(target, positionsBuffer);
		glVertexAttribPointer(positionAttribIdx, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	void GenericSubBuffers::createColorsBuffer(unsigned target)
	{
		assert(!colorsBuffer);
		colorsBuffer = 0;
		glGenBuffers(1, &*colorsBuffer);
		glBindBuffer(target, *colorsBuffer);
		glVertexAttribPointer(colorAttribIdx, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	void GenericSubBuffers::createVelocitiesAndTimesBuffer(unsigned target)
	{
		assert(!velocitiesAndTimesBuffer);
		velocitiesAndTimesBuffer = 0;
		glGenBuffers(1, &*velocitiesAndTimesBuffer);
		glBindBuffer(target, *velocitiesAndTimesBuffer);
		glVertexAttribPointer(velocityAndTimeAttribIdx, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	void GenericSubBuffers::createHSizesAndAnglesBuffer(unsigned target)
	{
		assert(!hSizesAndAnglesBuffer);
		hSizesAndAnglesBuffer = 0;
		glGenBuffers(1, &*hSizesAndAnglesBuffer);
		glBindBuffer(target, *hSizesAndAnglesBuffer);
		glVertexAttribPointer(hSizeAndAngleAttribIdx, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	void GenericSubBuffers::createTexCoordsBuffer(unsigned target)
	{
		assert(!texCoordsBuffer);
		texCoordsBuffer = 0;
		glGenBuffers(1, &*texCoordsBuffer);
		glBindBuffer(target, *texCoordsBuffer);
		glVertexAttribPointer(texCoordAttribIdx, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	void GenericSubBuffers::createNormalsBuffer(unsigned target)
	{
		assert(!normalsBuffer);
		normalsBuffer = 0;
		glGenBuffers(1, &*normalsBuffer);
		glBindBuffer(target, *normalsBuffer);
		glVertexAttribPointer(normalAttribIdx, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	void GenericSubBuffers::createInstancedTransformsBuffer(unsigned target)
	{
		assert(!instancedTransformsBuffer);
		instancedTransformsBuffer = 0;
		glGenBuffers(1, &*instancedTransformsBuffer);
		glBindBuffer(target, *instancedTransformsBuffer);
		for (unsigned i = 0; i < 4; ++i)
		{
			glVertexAttribPointer(instancedTransformAttribIdx + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
			glVertexAttribDivisor(instancedTransformAttribIdx + i, 1);
		}
	}

	void GenericSubBuffers::createInstancedNormalTransformsBuffer(unsigned target)
	{
		assert(!instancedNormalTransformsBuffer);
		instancedNormalTransformsBuffer = 0;
		glGenBuffers(1, &*instancedNormalTransformsBuffer);
		glBindBuffer(target, *instancedNormalTransformsBuffer);
		for (unsigned i = 0; i < 3; ++i)
		{
			glVertexAttribPointer(instancedNormalTransformAttribIdx + i, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(sizeof(glm::vec3) * i));
			glVertexAttribDivisor(instancedNormalTransformAttribIdx + i, 1);
		}
	}

	void GenericSubBuffers::createIndicesBuffer()
	{
		assert(!indicesBuffer);
		indicesBuffer = 0;
		glGenBuffers(1, &*indicesBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indicesBuffer);
	}

	GenericBuffers::GenericBuffers(bool defaultVAO) :
		GenericSubBuffers(defaultVAO)
	{
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

	void GenericBuffers::bindActiveTFBuffers() const
	{
		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, positionsBuffer);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, positionAttribIdx, positionsBuffer);

		if (colorsBuffer)
		{
			glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, *colorsBuffer);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, colorAttribIdx, *colorsBuffer);
		}

		if (velocitiesAndTimesBuffer)
		{
			glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, *velocitiesAndTimesBuffer);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, velocityAndTimeAttribIdx, *velocitiesAndTimesBuffer);
		}

		if (hSizesAndAnglesBuffer)
		{
			glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, *hSizesAndAnglesBuffer);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, hSizeAndAngleAttribIdx, *hSizesAndAnglesBuffer);
		}

		if (texCoordsBuffer)
		{
			glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, *texCoordsBuffer);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, texCoordAttribIdx, *texCoordsBuffer);
		}

		if (normalsBuffer)
		{
			glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, *normalsBuffer);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, normalAttribIdx, *normalsBuffer);
		}

		if (instancedTransformsBuffer)
		{
			glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, *instancedTransformsBuffer);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, instancedTransformAttribIdx, *instancedTransformsBuffer);
		}

		if (instancedNormalTransformsBuffer)
		{
			glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, *instancedNormalTransformsBuffer);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, instancedNormalTransformAttribIdx, *instancedNormalTransformsBuffer);
		}
	}
}
