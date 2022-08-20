#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>

namespace Components
{
	struct Gamepad : ComponentBase
	{
		bool enabled = false;

		struct Buttons
		{
			bool a = false;
			bool b = false;
			bool x = false;
			bool y = false;

			bool back = false;
			bool start = false;

			bool lStick = false;
			bool rStick = false;

			bool lShoulder = false;
			bool rShoulder = false;

			bool dUp = false;
			bool dDown = false;
			bool dLeft = false;
			bool dRight = false;
		} pressed, released, pressing;

		glm::vec2 lStick{ 0.0f };
		glm::vec2 rStick{ 0.0f };

		float lTrigger = 0.0f;
		float rTrigger = 0.0f;
	};
}
