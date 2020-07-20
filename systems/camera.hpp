#pragma once

#include <glm/vec2.hpp>

namespace Systems
{
	class Camera
	{
	public:
		Camera();

		void step();

	private:
		bool firstStep = true;
	};
}
