#include "graphicsHelpers.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "utility.hpp"

inline glm::vec2 OrthoVec2(const glm::vec2& p1, const glm::vec2& p2, bool invert = false)
{
	const glm::vec2 d = p2 - p1;
	if (!invert) return glm::normalize(glm::vec2( -d.y, d.x ));
	else return glm::normalize(glm::vec2(d.y, -d.x));
}

namespace Tools
{
	std::vector<glm::vec3> CreateCircleVertices(const glm::vec2& position, float radius, int complexity, const glm::mat4& modelMatrix)
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

	std::vector<glm::vec3> CreateLightningVertices(const glm::vec2& p1, const glm::vec2& p2, int segmentsNum, float frayFactor, float zValue)
	{
		std::vector<glm::vec3> result;
		result.reserve(segmentsNum * 2);

		const glm::vec2 d = glm::normalize(p2 - p1);
		const glm::vec2 step = (p2 - p1) / (float)segmentsNum;
		const float stepLength = glm::length(step);
		const glm::vec2 orthoD = OrthoVec2(p1, p2);
		glm::vec2 currentPos = p1;

		result.emplace_back(currentPos, zValue);
		for (int i = 0; i < segmentsNum; ++i)
		{
			const float variationStep = stepLength * Random(-frayFactor, frayFactor);

			currentPos += step;
			currentPos += orthoD * variationStep;

			result.emplace_back(currentPos, zValue);
			if (i != segmentsNum - 1) result.push_back(result.back());
		}

		const glm::vec2 delta = p2 - glm::vec2(result.back());
		const glm::vec2 stepCorrection = delta / (float)segmentsNum;

		for (int i = 0; i < segmentsNum; ++i)
		{
			result[i * 2] += glm::vec3(stepCorrection * (float)i, 0.0f);
			result[i * 2 + 1] += glm::vec3(stepCorrection * (float)(i + 1), 0.0f);
		}

		return result;
	}
}
