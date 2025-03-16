#include "Shapes2D.hpp"

#include "geometryHelpers.hpp"
#include "utility.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Tools::Shapes2D
{
	std::vector<glm::vec3> CreatePositionsOfRectangle(const glm::vec2& position, const glm::vec2& hSize, float angle, float z)
	{
		std::vector<glm::vec3> positions;
		positions.reserve(6);

		positions.emplace_back(-hSize, z);
		const auto& diagonal1 = positions.emplace_back(hSize.x, -hSize.y, z);
		const auto& diagonal2 = positions.emplace_back(-hSize.x, hSize.y, z);
		positions.push_back(diagonal2);
		positions.push_back(diagonal1);
		positions.emplace_back(hSize, z);

		if (angle != 0.0f)
		{
			glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), angle, { 0.0f, 0.0f, 1.0f });
			for (auto& localPosition : positions) localPosition = rotMat * glm::vec4(localPosition, 1.0f);
		}

		for (auto& localPosition : positions) localPosition += glm::vec3(position, z);

		return positions;
	}

	std::vector<glm::vec2> CreateTexCoordOfRectangle(const glm::vec2& leftDown, const glm::vec2& rightUp)
	{
		return {
			leftDown,
			{ rightUp.x, leftDown.y },
			{ leftDown.x, rightUp.y },
			{ leftDown.x, rightUp.y },
			{ rightUp.x, leftDown.y },
			rightUp
		};
	}

	std::vector<glm::vec3> CreatePositionsOfLineOfRectangles(const glm::vec2& hSize, const std::vector<glm::vec2>& positionsRanges,
		const glm::vec2& scaleRange, const glm::vec2& angleRange, const glm::vec2& stepRange, float z)
	{
		assert(positionsRanges.size() > 1);

		std::vector<glm::vec3> positions;

		for (auto it = positionsRanges.begin(); it != std::prev(positionsRanges.end()); ++it)
		{
			const auto& currentControlPos = *it;
			const auto& nextControlPos = *std::next(it);
			const std::vector<glm::vec3> rectangleVertices = CreatePositionsOfRectangle({ 0.0f, 0.0f }, hSize, 0.0f, z);
			const glm::vec2 direction = glm::normalize(nextControlPos - currentControlPos);
			const float lineLength = glm::distance(currentControlPos, nextControlPos);
			glm::vec2 currentPos = currentControlPos;

			do
			{
				const float scale = Tools::RandomFloat(scaleRange.x, scaleRange.y);
				const glm::mat4 transformation = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(currentPos, z)),
					Tools::RandomFloat(angleRange.x, angleRange.y), glm::vec3(0.0f, 0.0f, 1.0f)), { scale, scale, 1.0f });
				const std::vector<glm::vec3> transformedRectanglePositions = Tools::TransformMat4(rectangleVertices, transformation);
				positions.insert(positions.end(), transformedRectanglePositions.begin(), transformedRectanglePositions.end());
				currentPos += direction * Tools::RandomFloat(stepRange.x, stepRange.y);
			} while (glm::distance(currentControlPos, currentPos) < lineLength);
		}

		return positions;
	}

	std::vector<glm::vec3> CreatePositionsOfFunctionalRectangles(const glm::vec2& hSize, std::function<glm::vec2(float)> positionF,
		std::function<glm::vec2(float)> scaleF, std::function<float(float)> angleF, std::function<std::optional<float>()> inputEmitter, float z)
	{
		const std::vector<glm::vec3> rectangleVertices = CreatePositionsOfRectangle({ 0.0f, 0.0f }, hSize, 0.0f, z);
		std::vector<glm::vec3> positions;
		std::optional<float> input = inputEmitter();

		while (input)
		{
			const glm::vec2 position = positionF(*input);
			const glm::vec2 scale = scaleF(*input);
			const float angle = angleF(*input);
			const glm::mat4 transformation = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(position, z)),
				angle, glm::vec3(0.0f, 0.0f, 1.0f)), { scale.x, scale.y, 1.0f });
			const std::vector<glm::vec3> transformedRectangleVertices = Tools::TransformMat4(rectangleVertices, transformation);
			positions.insert(positions.end(), transformedRectangleVertices.begin(), transformedRectangleVertices.end());
			input = inputEmitter();
		};

		return positions;
	}

	size_t AppendPositionsOfCircle(std::vector<glm::vec3>& positions, const glm::vec2& position, float radius, int complexity,
		const glm::mat4& modelMatrix, float z)
	{
		const size_t initialSize = positions.size();

		positions.reserve(positions.size() + complexity * 3);

		const float radialStep = glm::two_pi<float>() / complexity;

		for (int i = 0; i < complexity; ++i)
		{
			const float radialPosition = i * radialStep;
			const float nextRadialPosition = (i + 1) * radialStep;

			positions.push_back(modelMatrix * glm::vec4(position, z, 1.0f));
			positions.push_back(modelMatrix * glm::vec4(position + glm::vec2(glm::cos(radialPosition),
				glm::sin(radialPosition)) * radius, z, 1.0f));
			positions.push_back(modelMatrix * glm::vec4(position + glm::vec2(glm::cos(nextRadialPosition),
				glm::sin(nextRadialPosition)) * radius, z, 1.0f));
		}

		return positions.size() - initialSize;
	}

	std::vector<glm::vec3> CreatePositionsOfCircle(const glm::vec2& position, float radius, int complexity,
		const glm::mat4& modelMatrix, float z)
	{
		std::vector<glm::vec3> positions;
		AppendPositionsOfCircle(positions, position, radius, complexity, modelMatrix, z);
		return positions;
	}

	size_t AppendPositionsOfLightning(std::vector<glm::vec3>& positions, const glm::vec2& p1, const glm::vec2& p2,
		int segmentsNum, float frayFactor, float z)
	{
		// TODO: Investigate why this reserve is slowing down the program.
		//positions.reserve(positions.size() + segmentsNum * 2);

		const size_t initialSize = positions.size();
		const glm::vec2 d = glm::normalize(p2 - p1);
		const glm::vec2 step = (p2 - p1) / (float)segmentsNum;
		const float stepLength = glm::length(step);
		const glm::vec2 orthoD = Tools::OrthoVec2(p1, p2);
		glm::vec2 currentPos = p1;

		positions.emplace_back(currentPos, z);
		for (int i = 0; i < segmentsNum; ++i)
		{
			const float variationStep = stepLength * Tools::RandomFloat(-frayFactor, frayFactor);

			currentPos += step;
			currentPos += orthoD * variationStep;

			positions.emplace_back(currentPos, z);
			if (i < segmentsNum - 1) positions.push_back(positions.back());
		}

		const glm::vec2 delta = p2 - glm::vec2(positions.back());
		const glm::vec2 stepCorrection = delta / (float)segmentsNum;

		for (int i = 0; i < segmentsNum; ++i)
		{
			positions[initialSize + i * 2] += glm::vec3(stepCorrection * (float)i, 0.0f);
			positions[initialSize + i * 2 + 1] += glm::vec3(stepCorrection * (float)(i + 1), 0.0f);
		}

		return positions.size() - initialSize;
	}

	std::vector<glm::vec3> CreatePositionsOfLightning(const glm::vec2& p1, const glm::vec2& p2,
		int segmentsNum, float frayFactor, float z)
	{
		std::vector<glm::vec3> positions;
		AppendPositionsOfLightning(positions, p1, p2, segmentsNum, frayFactor, z);
		return positions;
	}
}
