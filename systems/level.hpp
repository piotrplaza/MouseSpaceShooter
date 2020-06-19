#pragma once

#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "shaders.hpp"

namespace Systems
{
	class Level
	{
	public:
		Level();

		void step();
		void renderBackground() const;
		void renderForeground() const;

	private:
		void initPhysics() const;
		void initGraphics();

		void updateWalls();
		void updateGrapples();
		void updateConnections();

		shaders::ProgramId basicShadersProgram;
		shaders::ProgramId coloredShadersProgram;

		GLuint wallsVertexArray;
		GLuint wallsVertexBuffer;

		GLuint grapplesVertexArray;
		GLuint grapplesVertexBuffer;

		GLuint connectionsVertexArray;
		GLuint connectionsVertexBuffer;
		GLuint connectionsColorBuffer;

		std::vector<glm::vec3> wallsVerticesCache;
		std::vector<glm::vec3> grapplesVerticesCache;
		std::vector<glm::vec3> connectionsVerticesCache;
		std::vector<glm::vec4> connectionsColorsCache;
	};
}
