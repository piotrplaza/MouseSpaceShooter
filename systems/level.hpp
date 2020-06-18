#pragma once

#include <vector>

#include <glm/vec3.hpp>

#include "shaders.hpp"

namespace Systems
{
	class Level
	{
	public:
		Level();

		void step();
		void render() const;

	private:
		void initPhysics() const;
		void initGraphics();

		void updateWalls();
		void updateGrapples();

		shaders::ProgramId shadersProgram;

		GLuint wallsVertexArray;
		GLuint wallsVertexBuffer;

		GLuint grapplesVertexArray;
		GLuint grapplesVertexBuffer;

		std::vector<glm::vec3> wallsVerticesCache;
		std::vector<glm::vec3> grapplesVerticesCache;
	};
}
