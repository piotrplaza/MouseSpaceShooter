#include "graphicsHelpers.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace tools
{
	std::vector<glm::vec3> createCircleVertices(glm::vec2 position, float radius, int complexity, const glm::mat4& modelMatrix)
	{
		std::vector<glm::vec3> vertices;
		vertices.reserve(complexity * 3);

		const float radialStep = glm::two_pi<float>() / complexity;

		for (int i = 0; i < complexity; ++i)
		{
			const float radialPosition = i * radialStep;
			const float nextRadialPosition = (i + 1) * radialStep;

			vertices.push_back(modelMatrix * glm::vec4(position, 0.0f, 1.0f));
			vertices.push_back(modelMatrix * glm::vec4(position + glm::vec2(glm::cos(radialPosition),
				glm::sin(radialPosition)) * radius, 0.0f, 1.0f));
			vertices.push_back(modelMatrix * glm::vec4(position + glm::vec2(glm::cos(nextRadialPosition),
				glm::sin(nextRadialPosition)) * radius, 0.0f, 1.0f));
		}

		return vertices;
	}
}
