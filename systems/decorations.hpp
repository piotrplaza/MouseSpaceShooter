#pragma once

#include <memory>
#include <optional>
#include <functional>
#include <vector>
#include <unordered_map>

#include <glm/vec3.hpp>

#include <ogl/shaders.hpp>
#include <shaders/basic.hpp>
#include <shaders/textured.hpp>

#include <buffers/posTexCoordBuffers.hpp>

namespace Systems
{
	class Decorations
	{
	public:
		Decorations();

		void renderBackground() const;
		void renderMidground() const;
		void renderForeground() const;

	private:
		void initGraphics();

		void updatePositionsBuffers();
		void updateTexCoordsBuffers();

		void texturedRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const;
		void basicRender(const std::vector<Buffers::PosTexCoordBuffers>& buffer) const;

		std::unique_ptr<Shaders::Programs::Basic> basicShadersProgram;
		std::unique_ptr<Shaders::Programs::Textured> texturedShadersProgram;

		std::vector<Buffers::PosTexCoordBuffers> simpleBackgroundDecorationsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> simpleMidgroundDecorationsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> simpleForegroundDecorationsBuffers;

		std::vector<Buffers::PosTexCoordBuffers> texturedBackgroundDecorationsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> texturedMidgroundDecorationsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> texturedForegroundDecorationsBuffers;
	};
}
