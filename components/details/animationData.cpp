#include "animationData.hpp"

#include <components/physics.hpp>
#include <globals/components.hpp>

#include <tools/glmHelpers.hpp>
#include <tools/utility.hpp>

#include <glm/gtc/matrix_transform.hpp>

AnimationData::AnimationData(CM::Texture texture, glm::ivec2 textureSize, glm::ivec2 framesGrid, glm::ivec2 leftTopFrameLeftTopCorner, int rightTopFrameLeftEdge, int leftBottomFrameTopEdge,
	glm::ivec2 frameSize, float frameDuration, int numOfFrames, int startFrame, Direction animationDirection, Mode animationMode, TextureLayout textureLayout) :
	texture(std::move(texture)),
	framesGrid(glm::max(framesGrid.x, 2), glm::max(framesGrid.y, 2)),
	leftTopFrameLeftTopCorner(glm::vec2(leftTopFrameLeftTopCorner) / textureSize),
	rightTopFrameLeftEdge((float)rightTopFrameLeftEdge / textureSize.x),
	leftBottomFrameTopEdge((float)leftBottomFrameTopEdge / textureSize.y),
	frameScale(glm::vec2(textureSize) / frameSize),
	frameDuration(frameDuration),
	numOfFrames(numOfFrames),
	startFrame(startFrame),
	animationDirection(animationDirection),
	animationMode(animationMode),
	textureLayout(textureLayout),
	hFrameSize(glm::vec2(frameSize) / textureSize)
{
	assert(startFrame >= 0 && startFrame < numOfFrames);
}

const CM::Texture& AnimationData::getTexture() const
{
	return texture;
}

glm::mat4 AnimationData::getFrameTransformation() const
{
	const auto frameLocation = getFrameLocation();
	const glm::vec2 delta((rightTopFrameLeftEdge - leftTopFrameLeftTopCorner.x) / (framesGrid.x - 1),
		(leftBottomFrameTopEdge - leftTopFrameLeftTopCorner.y) / (framesGrid.y - 1));
	const auto hFrameSize = 1.0f / frameScale * 0.5f;
	const auto translate = glm::translate(glm::mat4(1.0f), glm::vec3(leftTopFrameLeftTopCorner +
		glm::vec2(delta.x * frameLocation.x + hFrameSize.x - 0.5f, -delta.y * frameLocation.y - hFrameSize.y + 0.5f), 0.0f));

	return glm::scale(translate, glm::vec3(1.0f / frameScale, 1.0f)) * additionalTransform;
}

void AnimationData::start(bool value)
{
	started = value;
	animationTime = 0.0f;
	paused = false;
	prevSimDuration = started * Globals::Components().physics().simulationDuration;
}

bool AnimationData::isStarted() const
{
	return started;
}

void AnimationData::pause(bool value)
{
	paused = value;
}

bool AnimationData::isPaused() const
{
	return paused;
}

void AnimationData::setFrame(int frame)
{
	animationTime = frame * frameDuration;
}

void AnimationData::setSpeedScaling(float speedScaling)
{
	this->speedScaling = speedScaling;
}

void AnimationData::setAdditionalTransformation(glm::vec2 translate, float angle, glm::vec2 scale)
{
	additionalTransform = Tools::TextureTransform(translate, angle, scale);
}

void AnimationData::forceFrame(std::optional<int> frame)
{
	forcedFrame = frame;
}

bool AnimationData::isForcingFrame() const
{
	return forcedFrame.has_value();
}

int AnimationData::getAbsoluteFrame() const
{
	if (!started)
		return 0;

	animationTime += !paused * (Globals::Components().physics().simulationDuration - prevSimDuration) * speedScaling;
	prevSimDuration = Globals::Components().physics().simulationDuration;

	return int(animationTime / frameDuration);
}

int AnimationData::getCurrentFrame() const
{
	if (forcedFrame)
		return *forcedFrame;

	const int absoluteFrame = getAbsoluteFrame();
	const int absoluteFrameWithMode = [&]()
		{
			auto pingpong = [&]()
				{
					const bool backward = (absoluteFrame - 1) / (numOfFrames - 1) % 2;
					if (backward)
						return (absoluteFrame - 1) / (numOfFrames - 1) * (numOfFrames - 1) - absoluteFrame + numOfFrames - 1;
					else
						return absoluteFrame - (absoluteFrame - 1) / (numOfFrames - 1) * (numOfFrames - 1);
				};

			switch (animationMode)
			{
			case Mode::Repeat:
				return absoluteFrame;
			case Mode::Pingpong:
				return pingpong();
			case Mode::StopOnLastFrame:
				return absoluteFrame >= numOfFrames
					? numOfFrames - 1
					: absoluteFrame;
			default:
				assert(!"not implemented");
				return 0;
			}
		}();

	return ((((int)animationDirection * -2 + 1) * absoluteFrameWithMode + startFrame) % numOfFrames
		+ (int)animationDirection * numOfFrames) % numOfFrames;
}

glm::ivec2 AnimationData::getFrameLocation() const
{
	const int currentFrame = getCurrentFrame();
	return { currentFrame % framesGrid.x, currentFrame / framesGrid.x };
}
