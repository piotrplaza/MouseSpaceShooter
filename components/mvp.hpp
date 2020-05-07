#pragma once

#include <glm/mat4x4.hpp>

namespace Components
{
	struct MVP
	{
		glm::mat4 view{ 1.0f };
		glm::mat4 projection{ 1.0f };

		glm::mat4 getVP() const
		{
			return projection * view;
		}

		glm::mat4 getMVP(const glm::mat4& model) const
		{
			return getVP() * model;
		}
	};
}
