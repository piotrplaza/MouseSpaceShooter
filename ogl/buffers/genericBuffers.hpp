#pragma once

#include <components/_renderable.hpp>

#include <globals/components.hpp>

#include <commonTypes/componentMappers.hpp>
#include <commonTypes/resolutionMode.hpp>

#include <ogl/shadersUtils.hpp>
#include <ogl/oglProxy.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <optional>
#include <functional>
#include <vector>
#include <deque>
#include <variant>

namespace Buffers
{
	struct GenericSubBuffers
	{
		GenericSubBuffers();
		GenericSubBuffers(const GenericSubBuffers&) = delete;
		GenericSubBuffers(GenericSubBuffers&& other) noexcept;

		~GenericSubBuffers();

		void setVerticesBuffer(const std::vector<glm::vec3>& vertices);
		void setColorsBuffer(const std::vector<glm::vec4>& colors);
		void setTexCoordsBuffer(const std::vector<glm::vec2>& texCoords);
		void setNormalsBuffer(const std::vector<glm::vec3>& normals);
		void setInstancedTransformsBuffer(const std::vector<glm::mat4>& transforms);
		void setInstancedNormalTransformsBuffer(const std::vector<glm::mat3>& transforms);
		void setIndicesBuffer(const std::vector<unsigned>& indices);

		bool isInstancingActive() const;
		bool isIndicingActive() const;

		RenderableDef* renderable = nullptr;

		GLuint vertexArray = 0;
		size_t drawCount = 0;
		size_t instanceCount = 0;

	private:
		void createPositionsBuffer();
		void createColorsBuffer();
		void createTexCoordsBuffer();
		void createNormalsBuffer();
		void createInstancedTransformsBuffer();
		void createInstancedNormalTransformsBuffer();
		void createIndicesBuffer();

		GLuint positionsBuffer = 0;
		std::optional<GLuint> colorsBuffer;
		std::optional<GLuint> texCoordsBuffer;
		std::optional<GLuint> normalsBuffer;
		std::optional<GLuint> instancedTransformsBuffer;
		std::optional<GLuint> instancedNormalTransformsBuffer;
		std::optional<GLuint> indicesBuffer;
		size_t numOfAllocatedVertices = 0;
		size_t numOfAllocatedColors = 0;
		size_t numOfAllocatedTexCoords = 0;
		size_t numOfAllocatedNormals = 0;
		size_t numOfAllocatedInstancedTransforms = 0;
		size_t numOfAllocatedInstancedNormalTransforms = 0;
		size_t numOfAllocatedIndices = 0;
		std::optional<GLenum> allocatedBufferDataUsage;

		bool expired = false;
	};

	struct GenericBuffers : GenericSubBuffers
	{
		GenericBuffers() = default;
		GenericBuffers(const GenericBuffers&) = delete;
		GenericBuffers(GenericBuffers&& other) noexcept;

		const std::vector<glm::mat3>& calcNormalTransforms(const std::vector<glm::mat4>& transforms);

		void applyComponent(Renderable& renderableComponent, bool staticComponent);

		template <typename GeneralSetup, typename PostSetup>
		void draw(ShadersUtils::ProgramId programId, GeneralSetup generalSetup, PostSetup postSetup) const
		{
			if (!renderable || renderable->state == ComponentState::Outdated || !renderable->isEnabled())
				return;

			auto setAndDraw = [&](const GenericSubBuffers& buffers)
			{
				if (!(buffers.renderable->renderF)())
					return;

				glProxyBindVertexArray(buffers.vertexArray);

				generalSetup(buffers);

				std::function<void()> renderingTeardown;

				if (buffers.renderable->renderingSetupF)
					renderingTeardown = buffers.renderable->renderingSetupF(programId);

				postSetup(buffers);

				if (isInstancingActive())
				{
					if (isIndicingActive())
						glDrawElementsInstanced(buffers.renderable->drawMode, buffers.drawCount, GL_UNSIGNED_INT, nullptr, instanceCount);
					else
						glDrawArraysInstanced(buffers.renderable->drawMode, 0, buffers.drawCount, instanceCount);
				}
				else
				{
					if (isIndicingActive())
						glDrawElements(buffers.renderable->drawMode, buffers.drawCount, GL_UNSIGNED_INT, nullptr);
					else
						glDrawArrays(buffers.renderable->drawMode, 0, buffers.drawCount);
				}

				if (renderingTeardown)
					renderingTeardown();
			};

			for (unsigned i = 0; i < subsequence.size(); ++i)
			{
				const unsigned id = (i + renderable->subsequenceBegin) % subsequence.size();
				if (id == renderable->posInSubsequence)
					setAndDraw(*this);
				setAndDraw(subsequence[id]);
			}

			if (subsequence.empty() || renderable->posInSubsequence == subsequence.size())
				setAndDraw(*this);
		}

		Renderable* renderable = nullptr;

		std::deque<GenericSubBuffers> subsequence;
		std::vector<glm::mat3> normalTransforms;
	};
}
