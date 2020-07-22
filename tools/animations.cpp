#include "animations.hpp"

#include "glmHelpers.hpp"

#include <globals.hpp>

#include <components/physics.hpp>

namespace Tools
{
	TextureAnimationController::TextureAnimationController(
		glm::ivec2 imageSize, glm::ivec2 startPosition, glm::ivec2 frameSize, glm::ivec2 framesGrid, glm::vec2 frameStep, float frameTime,
		int numOfFrames, AnimationLayout animationLayout, AnimationPlayback animationPlayback, AnimationPolicy animationPolicy, glm::vec2 scale):
		imageSize(imageSize),
		startPosition(startPosition),
		frameSize(frameSize),
		framesGrid(framesGrid),
		frameStep(frameStep),
		frameTime(frameTime),
		numOfFrames(numOfFrames == -1 ? framesGrid.x * framesGrid.y : numOfFrames),
		animationLayout(animationLayout),
		animationPlayback(animationPlayback),
		animationPolicy(animationPolicy),
		scale(scale),
		textureScale(imageSize / frameSize),
		frameScale(glm::vec2(frameSize) / imageSize)
	{
	}

	TextureAnimationController::FrameTransformation TextureAnimationController::getFrameTransformation() const
	{
		const auto animationTime = pauseTime
			? *pauseTime - *startTime
			: startTime
			? Globals::Components::physics.simulationTime - *startTime
			: 0.0f;

		int currentFrame = int(animationTime / frameTime) % numOfFrames;
		if (animationPlayback == AnimationPlayback::Backward) currentFrame = numOfFrames - currentFrame - 1;
		const glm::ivec2 currentFrameInGrid = animationLayout == AnimationLayout::Vertical
			? glm::ivec2{ currentFrame / framesGrid.y, currentFrame % framesGrid.y }
		: glm::ivec2{ currentFrame % framesGrid.x, currentFrame / framesGrid.x };
		const glm::vec2 imageCoord = startPosition + currentFrameInGrid * frameStep;
		return { -imageCoord / imageSize - frameScale * 0.5f, glm::vec2(-textureScale.x, textureScale.y) * scale };
	}

	void TextureAnimationController::start()
	{
		startTime = Globals::Components::physics.simulationTime;
		pauseTime = std::nullopt;
	}

	void TextureAnimationController::stop()
	{
		startTime = std::nullopt;
		pauseTime = std::nullopt;
	}

	void TextureAnimationController::pause()
	{
		if (!pauseTime && startTime) pauseTime = Globals::Components::physics.simulationTime;
	}

	void TextureAnimationController::resume()
	{
		if (pauseTime && startTime)
			*startTime += Globals::Components::physics.simulationTime - *pauseTime;
		pauseTime = std::nullopt;
	}
}
