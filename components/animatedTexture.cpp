#include "animatedTexture.hpp"

#include <tools/glmHelpers.hpp>
#include <tools/graphicsHelpers.hpp>

#include <tools/utility.hpp>

#include <ogl/renderingHelpers.hpp>

#include <components/physics.hpp>
#include <components/texture.hpp>

#include <globals/components.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace Components
{
	AnimatedTexture::AnimatedTexture(unsigned textureId, glm::ivec2 textureSize, glm::ivec2 framesGrid, glm::ivec2 leftTopFrameLeftTopCorner, int rightTopFrameLeftEdge, int leftBottomFrameTopEdge,
		glm::ivec2 frameSize, float frameDuration, int numOfFrames, int startFrame, AnimationDirection animationDirection, AnimationPolicy animationPolicy, TextureLayout textureLayout):
		textureId(textureId),
		framesGrid(framesGrid),
		leftTopFrameLeftTopCorner(glm::vec2(leftTopFrameLeftTopCorner) / textureSize),
		rightTopFrameLeftEdge((float)rightTopFrameLeftEdge / textureSize.x),
		leftBottomFrameTopEdge((float)leftBottomFrameTopEdge / textureSize.y),
		frameScale(glm::vec2(textureSize) / frameSize),
		frameDuration(frameDuration),
		numOfFrames(numOfFrames),
		startFrame(startFrame),
		animationDirection(animationDirection),
		animationPolicy(animationPolicy),
		textureLayout(textureLayout),
		hFrameSize(glm::vec2(frameSize) / textureSize)
	{
		assert(startFrame >= 0 && startFrame < numOfFrames);
	}

	unsigned AnimatedTexture::getTextureId() const
	{
		return textureId;
	}

	glm::mat4 AnimatedTexture::getFrameTransformation()
	{
		const auto frameLocation = getFrameLocation();
		const glm::vec2 delta((rightTopFrameLeftEdge - leftTopFrameLeftTopCorner.x) / (framesGrid.x - 1),
			(leftBottomFrameTopEdge - leftTopFrameLeftTopCorner.y) / (framesGrid.y - 1));
		const auto hFrameSize = 1.0f / frameScale * 0.5f;
		const auto translate = glm::translate(glm::mat4(1.0f), glm::vec3(leftTopFrameLeftTopCorner +
			glm::vec2(delta.x * frameLocation.x + hFrameSize.x - 0.5f, -delta.y * frameLocation.y - hFrameSize.y + 0.5f), 0.0f));

		return glm::scale(translate, glm::vec3(1.0f / frameScale, 1.0f)) * additionalTransform;
	}

	void AnimatedTexture::start(bool value)
	{
		started = value;
		animationTime = 0.0f;
		paused = false;
		prevSimDuration = started * Globals::Components().physics().simulationDuration;
	}

	bool AnimatedTexture::isStarted() const
	{
		return started;
	}

	void AnimatedTexture::pause(bool value)
	{
		paused = value;
	}

	bool AnimatedTexture::isPaused() const
	{
		return paused;
	}

	void AnimatedTexture::setSpeedScaling(float speedScaling)
	{
		this->speedScaling = speedScaling;
	}

	void AnimatedTexture::setAdditionalTransformation(glm::vec2 translate, float angle, glm::vec2 scale)
	{
		additionalTransform = Tools::TextureTransform(translate, angle, scale);
	}

	int AnimatedTexture::getAbsoluteFrame()
	{
		if (!started)
			return 0;

		animationTime += !paused * (Globals::Components().physics().simulationDuration - prevSimDuration) * speedScaling;
		prevSimDuration = Globals::Components().physics().simulationDuration;

		return int(animationTime / frameDuration);
	}

	int AnimatedTexture::getCurrentFrame()
	{
		const int absoluteFrame = getAbsoluteFrame();
		const int absoluteFrameWithPolicy = [&]()
		{
			auto pingpong = [&]()
			{
				const bool backward = (absoluteFrame - 1) / (numOfFrames - 1) % 2;
				if (backward)
					return (absoluteFrame - 1) / (numOfFrames - 1) * (numOfFrames - 1) - absoluteFrame + numOfFrames - 1;
				else
					return absoluteFrame - (absoluteFrame - 1) / (numOfFrames - 1) * (numOfFrames - 1);
			};

			switch (animationPolicy)
			{
			case AnimationPolicy::Repeat:
				return absoluteFrame;
			case AnimationPolicy::Pingpong:
				return pingpong();
			case AnimationPolicy::StopOnLastFrame:
				return absoluteFrame >= numOfFrames
					? numOfFrames - 1
					: absoluteFrame;
			default:
				assert(!"not implemented");
				return 0;
			}
		}();

		return ((((int)animationDirection * -2 + 1) * absoluteFrameWithPolicy + startFrame) % numOfFrames
			+ (int)animationDirection * numOfFrames) % numOfFrames;
	}

	glm::ivec2 AnimatedTexture::getFrameLocation()
	{
		const int currentFrame = getCurrentFrame();

		return { currentFrame % framesGrid.x, currentFrame / framesGrid.x };
	}
}
