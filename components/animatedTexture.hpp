#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <functional>
#include <optional>

enum class TextureLayout { Horizontal, Vertical };
enum class AnimationDirection { Forward, Backward };
enum class AnimationPolicy { Repeat, Pingpong, StopOnLastFrame };

namespace Components
{
	struct AnimatedTexture : ComponentBase
	{
		AnimatedTexture(unsigned textureId, glm::ivec2 textureSize, glm::ivec2 framesGrid, glm::ivec2 leftTopFrameLeftTopCorner, int rightTopFrameLeftEdge, int leftBottomFrameTopEdge,
			glm::ivec2 frameSize, float frameDuration, int numOfFrames = 0, int startFrame = 0, AnimationDirection animationDirection = AnimationDirection::Forward, 
			AnimationPolicy animationPolicy = AnimationPolicy::Repeat, TextureLayout textureLayout = TextureLayout::Horizontal);

		unsigned getTextureId() const;

		glm::mat4 getFrameTransformation();

		void start(bool value);
		bool isStarted() const;

		void pause(bool value);
		bool isPaused() const;

		void setSpeedScaling(float speedScaling);

		void setAdditionalTransformation(glm::vec2 translate, float angle = 0.0f, glm::vec2 scale = { 1.0f, 1.0f });

	private:
		int getAbsoluteFrame();
		int getCurrentFrame();
		glm::ivec2 getFrameLocation();

		const unsigned textureId{};
		const glm::ivec2 framesGrid{};
		const glm::vec2 leftTopFrameLeftTopCorner{};
		const float rightTopFrameLeftEdge{};
		const float leftBottomFrameTopEdge{};
		const glm::vec2 frameScale{};
		const float frameDuration{};
		const int numOfFrames{};
		const int startFrame{};
		const AnimationDirection animationDirection{};
		const AnimationPolicy animationPolicy{};
		const TextureLayout textureLayout{};
		const glm::vec2 hFrameSize{};

		float speedScaling{ 1.0f };

		bool started{};
		bool paused{};
		
		float prevSimDuration{};
		float animationTime{};

		glm::mat4 additionalTransform{ 1.0f };
	};
}
