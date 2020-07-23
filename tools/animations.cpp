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
		if (prevTime && !pauseTime)
		{
			animationTime += (Globals::Components::physics.simulationTime - *prevTime) * timeScale;
			prevTime = Globals::Components::physics.simulationTime;
		}

		int currentFrame = int(animationTime / frameTime) % numOfFrames;
		if (animationTime < 0.0f) currentFrame += numOfFrames - 1;
		if (animationPlayback == AnimationPlayback::Backward) currentFrame = numOfFrames - currentFrame - 1;
		const glm::ivec2 currentFrameInGrid = animationLayout == AnimationLayout::Vertical
			? glm::ivec2{ currentFrame / framesGrid.y, currentFrame % framesGrid.y }
		: glm::ivec2{ currentFrame % framesGrid.x, currentFrame / framesGrid.x };
		const glm::vec2 imageCoord = startPosition + currentFrameInGrid * frameStep;
		return { -imageCoord / imageSize - frameScale * 0.5f, glm::vec2(-textureScale.x, textureScale.y) * scale };
	}

	void TextureAnimationController::start()
	{
		animationTime = 0;
		prevTime = Globals::Components::physics.simulationTime;
		pauseTime = std::nullopt;
	}

	void TextureAnimationController::stop()
	{
		prevTime = std::nullopt;
		pauseTime = std::nullopt;
	}

	void TextureAnimationController::pause()
	{
		if (!pauseTime && prevTime) pauseTime = Globals::Components::physics.simulationTime;
	}

	void TextureAnimationController::resume()
	{
		if (pauseTime && prevTime)
			*prevTime += Globals::Components::physics.simulationTime - *pauseTime;
		pauseTime = std::nullopt;
	}

	void TextureAnimationController::setTimeScale(float timeScale)
	{
		this->timeScale = timeScale;
	}

	float TextureAnimationController::getTimeScale() const
	{
		return timeScale;
	}
}
