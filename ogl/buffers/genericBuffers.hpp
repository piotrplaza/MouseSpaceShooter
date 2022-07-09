#pragma once

#include <ogl/shaders.hpp>

#include <commonTypes/resolutionMode.hpp>
#include <components/_typeComponentMappers.hpp>
#include <components/_typeComponentMappers.hpp>
#include <components/renderingSetup.hpp>

#include <globals/components.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <optional>
#include <functional>
#include <vector>
#include <variant>

namespace Buffers
{
	struct GenericSubBuffers
	{
		GenericSubBuffers();
		GenericSubBuffers(const GenericSubBuffers&) = delete;
		GenericSubBuffers(GenericSubBuffers&& other) noexcept;

		~GenericSubBuffers();

		void allocateOrUpdateVerticesBuffer(const std::vector<glm::vec3>& vertices);
		void allocateOrUpdateColorsBuffer(const std::vector<glm::vec4>& colors);
		void allocateOrUpdateTexCoordBuffer(const std::vector<glm::vec2>& texCoord);

		std::function<glm::mat4()> modelMatrixF;
		TextureComponentVariant texture;
		std::optional<ComponentId> renderingSetup;
		std::optional<Shaders::ProgramId> customShadersProgram;
		ResolutionMode resolutionMode = ResolutionMode::Normal;
		GLenum drawMode = GL_TRIANGLES;
		GLenum bufferDataUsage = GL_STATIC_DRAW;
		bool preserveTextureRatio = false;
		GLuint vertexArray;
		size_t numOfVertices = 0;
		bool render = true;

	private:
		void createColorBuffer();
		void createTexCoordBuffer();

		GLuint positionBuffer;
		std::optional<GLuint> colorBuffer;
		std::optional<GLuint> texCoordBuffer;
		size_t numOfAllocatedVertices = 0;
		size_t numOfAllocatedColors = 0;
		size_t numOfAllocatedTexCoord = 0;
		std::optional<GLenum> allocatedBufferDataUsage;

		bool expired = false;
	};

	struct GenericBuffers : GenericSubBuffers
	{
		template <typename GeneralSetup>
		void draw(Shaders::ProgramId programId, GeneralSetup generalSetup) const
		{
			auto setAndDraw = [&](const GenericSubBuffers& buffers)
			{
				if (!buffers.render)
					return;

				glBindVertexArray(buffers.vertexArray);

				generalSetup(buffers);

				std::function<void()> renderingTeardown;
				if (buffers.renderingSetup)
					renderingTeardown = Globals::Components().renderingSetups()[*buffers.renderingSetup](programId);

				glDrawArrays(buffers.drawMode, 0, buffers.numOfVertices);

				if (renderingTeardown)
					renderingTeardown();
			};

			for (auto it = subsequence.begin(); it != std::next(subsequence.begin(), posInSubsequence); ++it)
				setAndDraw(*it);

			setAndDraw(*this);

			for (auto it = std::next(subsequence.begin(), posInSubsequence); it != subsequence.end(); ++it)
				setAndDraw(*it);
		}

		std::vector<GenericSubBuffers> subsequence;
		unsigned posInSubsequence = 0;

		ComponentId sourceComponent = 0;
	};
}
