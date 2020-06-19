#pragma once

#include "shaders.hpp"

#include <glm/vec2.hpp>

namespace Systems
{
	class Player
	{
	public:
		Player();

		void step();
		void render() const;

	private:
		void initPhysics() const;
		void initGraphics();

		void turn(glm::vec2 controllerDelta) const;
		void throttle(bool active) const;
		void magneticHook(bool active) const;
		void createGrappleJoint() const;

		shaders::ProgramId basicShadersProgram;
		GLint basicShadersMVPUniform;

		GLuint vertexArray;
		GLuint vertexBuffer;

		bool firstStep = true;
	};
}
