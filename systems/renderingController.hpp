#pragma once

#include <memory>

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
	};
}
