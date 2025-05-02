#pragma once

#include "_componentBase.hpp"

#include <commonTypes/fTypes.hpp>

#include <glm/vec4.hpp>

#include <optional>

namespace Components
{
	struct GraphicsSettings : ComponentBase
	{
		std::optional<glm::ivec2> forcedResolution;
		FVec4 backgroundColorF = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
		FVec4 defaultColorF = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
		float pointSize{ 1.0f };
		float lineWidth{ 3.0f };
		std::optional<bool> forcedDepthTest;
		bool cullFace = true;
		bool pointSmooth = false;
		bool lineSmooth = false;
		bool force3D = false;
	};
}
