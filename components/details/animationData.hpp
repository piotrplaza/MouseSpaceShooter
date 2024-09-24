#pragma once

#include <commonTypes/componentMappers.hpp>

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

struct AnimationData
{
	enum class TextureLayout { Horizontal, Vertical };
	enum class Direction { Forward, Backward };
	enum class Mode { Repeat, Pingpong, StopOnLastFrame };

	AnimationData() = default;

	AnimationData(TextureComponentVariant texture, glm::ivec2 textureSize, glm::ivec2 framesGrid, glm::ivec2 leftTopFrameLeftTopCorner, int rightTopFrameLeftEdge, int leftBottomFrameTopEdge,
		glm::ivec2 frameSize, float frameDuration, int numOfFrames = 0, int startFrame = 0, Direction animationDirection = Direction::Forward,
		Mode animationMode = Mode::Repeat, TextureLayout textureLayout = TextureLayout::Horizontal);

	const TextureComponentVariant& getTexture() const;

	glm::mat4 getFrameTransformation() const;

	void start(bool value);
	bool isStarted() const;

	void pause(bool value);
	bool isPaused() const;

	void setSpeedScaling(float speedScaling);

	void setAdditionalTransformation(glm::vec2 translate, float angle = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

private:
	int getAbsoluteFrame() const;
	int getCurrentFrame() const;
	glm::ivec2 getFrameLocation() const;

	TextureComponentVariant texture{};
	glm::ivec2 framesGrid{};
	glm::vec2 leftTopFrameLeftTopCorner{};
	float rightTopFrameLeftEdge{};
	float leftBottomFrameTopEdge{};
	glm::vec2 frameScale{};
	float frameDuration{};
	int numOfFrames{};
	int startFrame{};
	Direction animationDirection{};
	Mode animationMode{};
	TextureLayout textureLayout{};
	glm::vec2 hFrameSize{};

	float speedScaling{ 1.0f };

	bool started{};
	bool paused{};

	mutable float prevSimDuration{};
	mutable float animationTime{};

	glm::mat4 additionalTransform{ 1.0f };
};