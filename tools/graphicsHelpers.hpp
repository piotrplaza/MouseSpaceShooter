#pragma once

#include <vector>
#include <functional>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace Tools
{
	std::vector<glm::vec3> CreateRectanglePositions(const glm::vec2& position, const glm::vec2& hSize);

	std::vector<glm::vec3> CreateCirclePositions(const glm::vec2& position, float radius, int complexity,
		const glm::mat4& modelMatrix = glm::mat4(1.0f));

	std::vector<glm::vec3> CreateLightningPositions(const glm::vec2& p1, const glm::vec2& p2,
		int segmentsNum, float frayFactor = 1.0f, float zValue = 0.0f);

	struct TextureFrameTransform
	{
		glm::vec2 translate;
		glm::vec2 scale;
	};

	enum class AnimationPolicy { StopOnLastFrame, Repeat, Pingpong };

	std::function<TextureFrameTransform(float time)> CreateTextureAnimation(
		glm::ivec2 imageSize, glm::ivec2 startPosition, glm::ivec2 frameSize, glm::ivec2 frameGrid, glm::vec2 frameStep,
		float frameTime, int numOfFrames = -1, bool verticalLayout = false, bool backward = false, AnimationPolicy animationPolicy = AnimationPolicy::Repeat);
}
