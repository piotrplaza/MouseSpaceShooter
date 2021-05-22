#include "animations.hpp"

#include "glmHelpers.hpp"

#include <globals.hpp>

#include <components/physics.hpp>

namespace Tools
{
	TextureAnimationController::TextureAnimationController(
		glm::ivec2 imageSize, glm::ivec2 startPosition, glm::ivec2 frameSize, glm::ivec2 framesGrid, glm::vec2 frameStep, float frameDuration,
		int numOfFrames, AnimationLayout animationLayout, AnimationPlayback animationPlayback, AnimationPolicy animationPolicy, glm::vec2 translate, glm::vec2 scale):
		imageSize(imageSize),
		startPosition(startPosition.x, -startPosition.y),
		framesGrid(framesGrid),
		frameStep(frameStep),
		frameDuration(frameDuration),
		numOfFrames(numOfFrames == 0 ? framesGrid.x * framesGrid.y : numOfFrames),
		animationLayout(animationLayout),
		animationPlayback(animationPlayback),
		animationPolicy(animationPolicy),
		translate(translate),
		scale(scale),
		textureScale(glm::vec2(imageSize) / frameSize),
		frameScale(glm::vec2(frameSize) / imageSize)
	{
	}

	TextureAnimationController::FrameTransformation TextureAnimationController::getFrameTransformation() const
	{
		if (prevDuration && !pauseDuration)
		{
			animationDuration += (Globals::Components::physics.simulationDuration - *prevDuration) * getDurationScale();
			prevDuration = Globals::Components::physics.simulationDuration;
		}

		int currentFrame = int(animationDuration / frameDuration) % numOfFrames;
		if (animationDuration < 0.0f) currentFrame += numOfFrames - 1;
		if (animationPlayback == AnimationPlayback::Backward) currentFrame = numOfFrames - currentFrame - 1;
		const glm::ivec2 currentFrameInGrid = animationLayout == AnimationLayout::Vertical
			? glm::ivec2{ currentFrame / framesGrid.y, currentFrame % framesGrid.y }
		: glm::ivec2{ currentFrame % framesGrid.x, currentFrame / framesGrid.x };
		const glm::vec2 imageCoord = startPosition + currentFrameInGrid * frameStep;
		const glm::vec2 frameTranslate =
			glm::vec2(0.0f, -1.0f) - glm::vec2(imageCoord.x, -imageCoord.y) / imageSize
			- glm::vec2(frameScale.x, -frameScale.y) * 0.5f + translate * frameScale;
		const glm::vec2 frameScale = glm::vec2(textureScale.x, textureScale.y) * scale;
		return { frameTranslate, frameScale };
	}

	void TextureAnimationController::start()
	{
		animationDuration = 0;
		prevDuration = Globals::Components::physics.simulationDuration;
		pauseDuration = std::nullopt;
	}

	void TextureAnimationController::stop()
	{
		prevDuration = std::nullopt;
		pauseDuration = std::nullopt;
	}

	void TextureAnimationController::pause()
	{
		if (!pauseDuration && prevDuration) pauseDuration = Globals::Components::physics.simulationDuration;
	}

	void TextureAnimationController::resume()
	{
		if (pauseDuration && prevDuration)
			*prevDuration += Globals::Components::physics.simulationDuration - *pauseDuration;
		pauseDuration = std::nullopt;
	}

	void TextureAnimationController::setDurationScale(float durationScale)
	{
		this->durationScale = durationScale;
	}

	float TextureAnimationController::getDurationScale() const
	{
		return durationScale;
	}
}
