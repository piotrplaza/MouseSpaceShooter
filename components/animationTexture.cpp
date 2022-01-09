#include "animationTexture.hpp"

#include <tools/glmHelpers.hpp>

#include <components/physics.hpp>

namespace Components
{
	AnimationTexture::AnimationTexture(
		unsigned textureId, glm::ivec2 imageSize, glm::ivec2 startPosition, glm::ivec2 frameSize, glm::ivec2 framesGrid, glm::vec2 frameStep, float frameDuration,
		int numOfFrames, AnimationLayout animationLayout, AnimationPlayback animationPlayback, AnimationPolicy animationPolicy, glm::vec2 translate, glm::vec2 scale) :
		textureId(textureId),
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

	unsigned AnimationTexture::getTextureId() const
	{
		return textureId;
	}

	AnimationTexture::FrameTransformation AnimationTexture::getFrameTransformation() const
	{
		if (prevDuration && !pauseDuration)
		{
			animationDuration += (Globals::Components().physics().simulationDuration - *prevDuration) * getDurationScale();
			prevDuration = Globals::Components().physics().simulationDuration;
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

	void AnimationTexture::start()
	{
		animationDuration = 0;
		prevDuration = Globals::Components().physics().simulationDuration;
		pauseDuration = std::nullopt;
	}

	void AnimationTexture::stop()
	{
		prevDuration = std::nullopt;
		pauseDuration = std::nullopt;
	}

	void AnimationTexture::pause()
	{
		if (!pauseDuration && prevDuration) pauseDuration = Globals::Components().physics().simulationDuration;
	}

	void AnimationTexture::resume()
	{
		if (pauseDuration && prevDuration)
			*prevDuration += Globals::Components().physics().simulationDuration - *pauseDuration;
		pauseDuration = std::nullopt;
	}

	void AnimationTexture::setDurationScale(float durationScale)
	{
		this->durationScale = durationScale;
	}

	float AnimationTexture::getDurationScale() const
	{
		return durationScale;
	}
}
