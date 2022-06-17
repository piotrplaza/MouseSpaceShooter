#pragma once

#include <ogl/shaders.hpp>

#include <commonTypes/resolutionMode.hpp>
#include <components/typeComponentMappers.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <optional>
#include <functional>
#include <vector>
#include <variant>

namespace Buffers
{
	struct GenericBuffers
	{
		GenericBuffers();
		GenericBuffers(const GenericBuffers&) = delete;
		GenericBuffers(GenericBuffers&& other) noexcept;

		~GenericBuffers();

		void allocateOrUpdatePositionsBuffer(const std::vector<glm::vec3>& vertices);
		void allocateOrUpdateColorsBuffer(const std::vector<glm::vec2>& colors);
		void allocateOrUpdateTexCoordBuffer(const std::vector<glm::vec2>& texCoord);

		void draw() const;

		std::function<glm::mat4()> modelMatrixF;
		TextureComponentVariant texture;
		ComponentId renderingSetup = 0;
		std::optional<Shaders::ProgramId> customShadersProgram;
		ResolutionMode resolutionMode = ResolutionMode::Normal;

		GLenum drawMode = GL_TRIANGLES;
		GLenum bufferDataUsage = GL_STATIC_DRAW;

		bool preserveTextureRatio = false;

	private:
		void createColorBuffer();
		void createTexCoordBuffer();

		GLuint vertexArray;
		GLuint positionBuffer;
		std::optional<GLuint> colorBuffer;
		std::optional<GLuint> texCoordBuffer;
		size_t numOfPositions = 0;
		size_t numOfColors = 0;
		size_t numOfTexCoord = 0;
		size_t numOfAllocatedPositions = 0;
		size_t numOfAllocatedColors = 0;
		size_t numOfAllocatedTexCoord = 0;
		std::optional<GLenum> allocatedBufferDataUsage;
		bool expired = false;
	};
}
