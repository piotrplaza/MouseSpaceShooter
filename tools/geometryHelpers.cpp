#include "geometryHelpers.hpp"

#include "utility.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Tools
{
	std::vector<glm::vec3>& InPlaceTransformMat4(std::vector<glm::vec3>& vertices, const glm::mat4& transform)
	{
		for (auto& vertex : vertices)
			vertex = transform * glm::vec4(vertex, 1.0f);

		return vertices;
	}

	std::vector<glm::vec3> TransformMat4(const std::vector<glm::vec3>& vertices, const glm::mat4& transform)
	{
		std::vector<glm::vec3> result(vertices.begin(), vertices.end());

		return InPlaceTransformMat4(result, transform);
	}

	std::vector<glm::vec3>& InPlaceTransformMat3(std::vector<glm::vec3>& vertices, const glm::mat3& transform)
	{
		for (auto& vertex : vertices)
			vertex = transform * vertex;

		return vertices;
	}

	std::vector<glm::vec3> TransformMat3(const std::vector<glm::vec3>& vertices, const glm::mat3& transform)
	{
		std::vector<glm::vec3> result(vertices.begin(), vertices.end());

		return InPlaceTransformMat3(result, transform);
	}

	glm::vec2 OrthoVec2(const glm::vec2& p1, const glm::vec2& p2, bool invert)
	{
		const glm::vec2 d = p2 - p1;
		if (!invert) return glm::normalize(glm::vec2(-d.y, d.x));
		else return glm::normalize(glm::vec2(d.y, -d.x));
	}

	void VerticesDefaultRandomTranslate(std::vector<glm::vec3>& vertices, bool loop, float randFactor)
	{
		for (size_t i = 0; i < vertices.size(); ++i)
		{
			if (!loop && (i == 0 || i == vertices.size() - 1))
				continue;

			glm::vec3& v = vertices[i];

			auto vTransform = [&](const glm::vec3& prevV, const glm::vec3& v, const glm::vec3& nextV) {
				const float avgLength = (glm::distance(prevV, v) + glm::distance(v, nextV)) * 0.5f;
				const float r = avgLength * randFactor;
				return glm::vec3(Tools::Random(-r, r), Tools::Random(-r, r), 0.0f);
				};

			if (loop && i == 0)
			{
				const glm::vec3& prevV = vertices[vertices.size() - 2];
				const glm::vec3& nextV = vertices[1];
				v += vTransform(prevV, v, nextV);
				continue;
			}

			if (loop && i == vertices.size() - 1)
			{
				v = vertices.front();
				continue;
			}

			const glm::vec3& prevV = vertices[i - 1];
			const glm::vec3& nextV = vertices[i + 1];
			v += vTransform(prevV, v, nextV);
		}
	}
}
