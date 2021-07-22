#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

#include <ogl/shaders/textured.hpp>

namespace Systems
{
	class RenderingController
	{
	public:
		RenderingController();

		void render();

	private:
		void initGraphics();

		std::unique_ptr<Shaders::Programs::Textured> texturedShadersProgram;

		glm::mat4 vp;
		glm::mat4 model;

		float angle;
	};
}
