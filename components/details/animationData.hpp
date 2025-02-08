#pragma once

#include <commonTypes/componentMappers.hpp>

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <optional>

struct AnimationData
{
	enum class TextureLayout { Horizontal, Vertical };
	enum class Direction { Forward, Backward };
	enum class Mode { Repeat, Pingpong, StopOnLastFrame };

	AnimationData() = default;

	AnimationData(CM::Texture texture, glm::ivec2 textureSize, glm::ivec2 framesGrid, glm::ivec2 leftTopFrameLeftTopCorner, int rightTopFrameLeftEdge, int leftBottomFrameTopEdge,
		glm::ivec2 frameSize, float frameDuration, int numOfFrames = 0, int startFrame = 0, Direction animationDirection = Direction::Forward,
		Mode animationMode = Mode::Repeat, TextureLayout textureLayout = TextureLayout::Horizontal);

	const CM::Texture& getTexture() const;

	glm::mat4 getFrameTransformation(float functionalSpeedScaling = 1.0f) const;

	void start(bool value);
	bool isStarted() const;

	void pause(bool value);
	bool isPaused() const;

	void setFrame(int frame);
	void setSpeedScaling(float speedScaling);

	void setAdditionalTransformation(glm::vec2 translate, float angle = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

	void forceFrame(std::optional<int> frame);
	bool isForcingFrame() const;

private:
	int getAbsoluteFrame(float functionalSpeedScaling) const;
	int getCurrentFrame(float functionalSpeedScaling) const;
	glm::ivec2 getFrameLocation(float functionalSpeedScaling) const;

	CM::Texture texture{};
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

	std::optional<int> forcedFrame;
};