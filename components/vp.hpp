#pragma once

#include "_componentBase.hpp"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace Components
{
	struct VP : ComponentBase
	{
		VP() = default;

		VP(glm::mat4 projection)
			: projection(projection)
		{
		}

		VP(glm::mat4 view, glm::mat4 projection)
			: view(view), projection(projection)
		{
		}

		glm::mat4 view{ 1.0f };
		glm::mat4 projection{ 1.0f };

		glm::mat4 getMV(const glm::mat4& model) const
		{
			return view * model;
		}

		glm::mat4 getVP() const
		{
			return projection * view;
		}

		glm::mat4 getMVP(const glm::mat4& model) const
		{
			return getVP() * model;
		}

		glm::mat3 getNormalMatrix(const glm::mat4& model) const
		{
			return glm::inverseTranspose(glm::mat3(/*getMV*/(model)));
		}

		glm::vec3 getViewPos() const
		{
			return glm::inverse(view)[3];
		}
	};
}
