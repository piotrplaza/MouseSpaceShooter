#include "graphicsHelpers.hpp"

#include "utility.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Details
{
	inline glm::vec2 OrthoVec2(const glm::vec2& p1, const glm::vec2& p2, bool invert = false)
	{
		const glm::vec2 d = p2 - p1;
		if (!invert) return glm::normalize(glm::vec2(-d.y, d.x));
		else return glm::normalize(glm::vec2(d.y, -d.x));
	}
}

namespace Tools
{
	std::vector<glm::vec3> CreateVerticesOfRectangle(const glm::vec2& position, const glm::vec2& hSize, float angle, float z)
	{
		std::vector<glm::vec3> vertices;
		vertices.reserve(6);

		vertices.emplace_back(-hSize, z);
		const auto& diagonal1 = vertices.emplace_back(hSize.x, -hSize.y, z);
		const auto& diagonal2 = vertices.emplace_back(-hSize.x, hSize.y, z);
		vertices.push_back(diagonal2);
		vertices.push_back(diagonal1);
		vertices.emplace_back(hSize, z);

		if (angle != 0.0f)
		{
			glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), angle, { 0.0f, 0.0f, 1.0f });
			for (auto& localPosition : vertices) localPosition = rotMat * glm::vec4(localPosition, 1.0f);
		}

		for (auto& localPosition : vertices) localPosition += glm::vec3(position, z);

		return vertices;
	}

	std::vector<glm::vec2> CreateTexCoordOfRectangle(const glm::vec2& leftDown, const glm::vec2& bottomUp)
	{
		return {
			leftDown,
			{ bottomUp.x, leftDown.y },
			{ leftDown.x, bottomUp.y },
			{ leftDown.x, bottomUp.y },
			{ bottomUp.x, leftDown.y },
			bottomUp
		};
	}

	std::vector<glm::vec3> CreateVerticesOfLineOfRectangles(const glm::vec2& hSize, const std::vector<glm::vec2>& positionsRanges,
		const glm::vec2& scaleRange, const glm::vec2& angleRange, const glm::vec2& stepRange, float z)
	{
		assert(positionsRanges.size() > 1);

		std::vector<glm::vec3> vertices;

		for (auto it = positionsRanges.begin(); it != std::prev(positionsRanges.end()); ++it)
		{
			const auto& currentControlPos = *it;
			const auto& nextControlPos = *std::next(it);
			const std::vector<glm::vec3> rectangleVertices = CreateVerticesOfRectangle({ 0.0f, 0.0f }, hSize, 0.0f, z);
			const glm::vec2 direction = glm::normalize(nextControlPos - currentControlPos);
			const float lineLength = glm::distance(currentControlPos, nextControlPos);
			glm::vec2 currentPos = currentControlPos;

			do
			{
				const float scale = Tools::Random(scaleRange.x, scaleRange.y);
				const glm::mat4 transformation = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(currentPos, z)),
					Tools::Random(angleRange.x, angleRange.y), glm::vec3(0.0f, 0.0f, 1.0f)), { scale, scale, 1.0f });
				const std::vector<glm::vec3> transformedRectanglePositions = Tools::Transform(rectangleVertices, transformation);
				vertices.insert(vertices.end(), transformedRectanglePositions.begin(), transformedRectanglePositions.end());
				currentPos += direction * Tools::Random(stepRange.x, stepRange.y);
			} while (glm::distance(currentControlPos, currentPos) < lineLength);
		}

		return vertices;
	}

	std::vector<glm::vec3> CreateVerticesOfFunctionalRectangles(const glm::vec2& hSize, std::function<glm::vec2(float)> positionF,
		std::function<glm::vec2(float)> scaleF, std::function<float(float)> angleF, std::function<std::optional<float>()> inputEmitter, float z)
	{
		const std::vector<glm::vec3> rectangleVertices = CreateVerticesOfRectangle({ 0.0f, 0.0f }, hSize, 0.0f, z);
		std::vector<glm::vec3> vertices;
		std::optional<float> input = inputEmitter();

		while (input)
		{
			const glm::vec2 position = positionF(*input);
			const glm::vec2 scale = scaleF(*input);
			const float angle = angleF(*input);
			const glm::mat4 transformation = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(position, z)),
				angle, glm::vec3(0.0f, 0.0f, 1.0f)), { scale.x, scale.y, 1.0f });
			const std::vector<glm::vec3> transformedRectangleVertices = Tools::Transform(rectangleVertices, transformation);
			vertices.insert(vertices.end(), transformedRectangleVertices.begin(), transformedRectangleVertices.end());
			input = inputEmitter();
		};

		return vertices;
	}

	void AppendVerticesOfCircle(std::vector<glm::vec3>& result, const glm::vec2& position, float radius, int complexity,
		const glm::mat4& modelMatrix, float z)
	{
		result.reserve(result.size() + complexity * 3);

		const float radialStep = glm::two_pi<float>() / complexity;

		for (int i = 0; i < complexity; ++i)
		{
			const float radialPosition = i * radialStep;
			const float nextRadialPosition = (i + 1) * radialStep;

			result.push_back(modelMatrix * glm::vec4(position,z, 1.0f));
			result.push_back(modelMatrix * glm::vec4(position + glm::vec2(glm::cos(radialPosition),
				glm::sin(radialPosition)) * radius, z, 1.0f));
			result.push_back(modelMatrix * glm::vec4(position + glm::vec2(glm::cos(nextRadialPosition),
				glm::sin(nextRadialPosition)) * radius, z, 1.0f));
		}
	}

	std::vector<glm::vec3> CreateVerticesOfCircle(const glm::vec2& position, float radius, int complexity,
		const glm::mat4& modelMatrix, float z)
	{
		std::vector<glm::vec3> vertices;
		AppendVerticesOfCircle(vertices, position, radius, complexity, modelMatrix, z);
		return vertices;
	}

	std::vector<glm::vec3> CreateVerticesOfLightning(const glm::vec2& p1, const glm::vec2& p2,
		int segmentsNum, float frayFactor, float z)
	{
		std::vector<glm::vec3> vertices;
		vertices.reserve(segmentsNum * 2);

		const glm::vec2 d = glm::normalize(p2 - p1);
		const glm::vec2 step = (p2 - p1) / (float)segmentsNum;
		const float stepLength = glm::length(step);
		const glm::vec2 orthoD = Details::OrthoVec2(p1, p2);
		glm::vec2 currentPos = p1;

		vertices.emplace_back(currentPos, z);
		for (int i = 0; i < segmentsNum; ++i)
		{
			const float variationStep = stepLength * Random(-frayFactor, frayFactor);

			currentPos += step;
			currentPos += orthoD * variationStep;

			vertices.emplace_back(currentPos, z);
			if (i != segmentsNum - 1) vertices.push_back(vertices.back());
		}

		const glm::vec2 delta = p2 - glm::vec2(vertices.back());
		const glm::vec2 stepCorrection = delta / (float)segmentsNum;

		for (int i = 0; i < segmentsNum; ++i)
		{
			vertices[i * 2] += glm::vec3(stepCorrection * (float)i, 0.0f);
			vertices[i * 2 + 1] += glm::vec3(stepCorrection * (float)(i + 1), 0.0f);
		}

		return vertices;
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

	std::vector<glm::vec3> Transform(const std::vector<glm::vec3>& vertices, const glm::mat4& transformation)
	{
		std::vector<glm::vec3> result(vertices.begin(), vertices.end());
		for (auto& vertex: result) vertex = transformation * glm::vec4(vertex, 1.0f);

		return result;
	}
}
