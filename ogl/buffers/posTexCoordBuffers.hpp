#pragma once

#include <optional>
#include <functional>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <ogl/shaders.hpp>

#include <tools/animations.hpp>

namespace Buffers
{
	struct PosTexCoordBuffers
	{
		PosTexCoordBuffers();
		PosTexCoordBuffers(const PosTexCoordBuffers&) = delete;
		PosTexCoordBuffers(PosTexCoordBuffers&& other) noexcept;

		~PosTexCoordBuffers();

		void createTexCoordBuffer();

		GLuint vertexArray;
		GLuint positionBuffer;

		std::optional<GLuint> texCoordBuffer;
		std::optional<unsigned> texture;
		const std::function<std::function<void()>(Shaders::ProgramId)>* renderingSetup = nullptr;
		const Tools::TextureAnimationController* animationController = nullptr;
		std::optional<Shaders::ProgramId> customShadersProgram;

		std::vector<glm::vec3> positionsCache;
		std::vector<glm::vec2> texCoordCache;
		size_t numOfAllocatedPositions = 0;
		size_t numOfAllocatedTexCoord = 0;

		GLenum drawMode = GL_TRIANGLES;
		GLenum bufferDataUsage = GL_STATIC_DRAW;
		std::optional<GLenum> allocatedBufferDataUsage;

		bool textureRatioPreserved = false;

		bool expired = false;
	};
}
