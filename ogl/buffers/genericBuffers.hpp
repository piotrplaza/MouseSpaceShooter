#pragma once

#include <ogl/shaders.hpp>

#include <components/_renderable.hpp>
#include <components/renderingSetup.hpp>

#include <globals/components.hpp>

#include <commonTypes/typeComponentMappers.hpp>
#include <commonTypes/resolutionMode.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
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

		std::function<glm::mat4()> modelMatrixF;
		std::function<glm::vec3()> originF;
		std::function<glm::vec4()> *colorF = nullptr;
		TextureComponentVariant* texture = nullptr;
		std::optional<ComponentId>* renderingSetup = nullptr;
		std::optional<Shaders::ProgramId>* customShadersProgram = nullptr;
		GLenum* drawMode = nullptr;
		GLenum* bufferDataUsage = nullptr;
		bool* preserveTextureRatio = nullptr;
		std::function<bool()>* renderF = nullptr;

		GLuint vertexArray = 0;
		size_t numOfVertices = 0;

	private:
		void createColorsBuffer();
		void createTexCoordsBuffer();
		void createNormalsBuffer();

		GLuint positionsBuffer = 0;
		std::optional<GLuint> colorsBuffer;
		std::optional<GLuint> texCoordsBuffer;
		std::optional<GLuint> normalsBuffer;
		size_t numOfAllocatedVertices = 0;
		size_t numOfAllocatedColors = 0;
		size_t numOfAllocatedTexCoords = 0;
		size_t numOfAllocatedNormals = 0;
		std::optional<GLenum> allocatedBufferDataUsage;

		bool expired = false;
	};

	struct GenericBuffers : GenericSubBuffers
	{
		void applyComponentSubsequence(Renderable& renderableComponent);
		void applyComponent(Renderable& renderableComponent);

		template <typename GeneralSetup>
		void draw(Shaders::ProgramId programId, GeneralSetup generalSetup) const
		{
			auto setAndDraw = [&](const GenericSubBuffers& buffers)
			{
				if (!(*buffers.renderF)())
					return;

				glBindVertexArray(buffers.vertexArray);

				generalSetup(buffers);

				std::function<void()> renderingTeardown;
				if (*buffers.renderingSetup)
					renderingTeardown = Globals::Components().renderingSetups()[**buffers.renderingSetup](programId);

				glDrawArrays(*buffers.drawMode, 0, buffers.numOfVertices);

				if (renderingTeardown)
					renderingTeardown();
			};

			for (unsigned i = 0; i < subsequence.size(); ++i)
			{
				const unsigned id = (i + *subsequenceBegin) % subsequence.size();
				if (id == *posInSubsequence)
					setAndDraw(*this);
				setAndDraw(subsequence[id]);
			}

			if (subsequence.empty() || *posInSubsequence == subsequence.size())
				setAndDraw(*this);
		}

		ResolutionMode* resolutionMode = nullptr;

		std::deque<GenericSubBuffers> subsequence;
		unsigned* subsequenceBegin = nullptr;
		unsigned* posInSubsequence = nullptr;
	};
}
