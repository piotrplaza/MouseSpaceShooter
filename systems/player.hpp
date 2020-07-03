#pragma once

#include "shaders.hpp"

#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Systems
{
	class Player
	{
	public:
		Player();

		void step();
		void render() const;
		void updateStaticPlayerGraphics() const;

	private:
		void initGraphics();

		void turn(glm::vec2 controllerDelta) const;
		void throttle(bool active) const;
		void magneticHook(bool active) const;
		void createGrappleJoint() const;

		void updateConnectionsGraphics();

		shaders::ProgramId basicShadersProgram;
		GLint basicShadersMVPUniform;
		GLint basicShadersColorUniform;

		shaders::ProgramId coloredShadersProgram;
		GLint coloredShadersMVPUniform;

		GLuint playerVertexArray;
		GLuint playerVertexBuffer;

		GLuint connectionsVertexArray;
		GLuint connectionsVertexBuffer;
		GLuint connectionsColorBuffer;

		std::vector<glm::vec3> connectionsVerticesCache;
		std::vector<glm::vec4> connectionsColorsCache;

		bool firstStep = true;
	};
}
