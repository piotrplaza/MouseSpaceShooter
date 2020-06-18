#pragma once

#include "shaders.hpp"

#include <glm/vec2.hpp>

namespace Systems
{
	class Player
	{
	public:
		Player();

		void step() const;
		void render() const;

	private:
		void initPhysics() const;
		void initGraphics();

		void turn(glm::vec2 controllerDelta) const;
		void throttle(bool active) const;
		void magneticHook(bool active) const;

		shaders::ProgramId shadersProgram;
		GLuint vertexArray;
		GLuint vertexBuffer;
	};
}
