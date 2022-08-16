#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>

namespace Components
{
	struct Gamepad : ComponentBase
	{
		bool enabled = false;

		bool aButton = false;
		bool bButton = false;
		bool xButton = false;
		bool yButton = false;
		bool backButton = false;
		bool startButton = false;
		bool lStickButton = false;
		bool rStickButton = false;
		bool lShoulderButton = false;
		bool rShoulderButton = false;

		bool dUpButton = false;
		bool dDownButton = false;
		bool dLeftButton = false;
		bool dRightButton = false;

		glm::vec2 lAxis{ 0.0f };
		glm::vec2 rAxis{ 0.0f };

		float lTrigger = 0.0f;
		float rTrigger = 0.0f;
	};
}
