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
		void updateStaticWallsGraphics();

	private:
		void initGraphics();

		void updateDynamicWallsGraphics();
		void updateGrapplesGraphics();
		void updateConnectionsGraphics();

		shaders::ProgramId basicShadersProgram;
		GLint basicShadersMVPUniform;
		GLint basicShadersColorUniform;

		shaders::ProgramId coloredShadersProgram;
		GLint coloredShadersMVPUniform;

		GLuint staticWallsVertexArray;
		GLuint staticWallsVertexBuffer;

		GLuint dynamicWallsVertexArray;
		GLuint dynamicWallsVertexBuffer;

		GLuint grapplesVertexArray;
		GLuint grapplesVertexBuffer;

		GLuint connectionsVertexArray;
		GLuint connectionsVertexBuffer;
		GLuint connectionsColorBuffer;

		std::vector<glm::vec3> staticWallsVerticesCache;
		std::vector<glm::vec3> dynamicWallsVerticesCache;
		std::vector<glm::vec3> grapplesVerticesCache;
		std::vector<glm::vec3> connectionsVerticesCache;
		std::vector<glm::vec4> connectionsColorsCache;
	};
}
