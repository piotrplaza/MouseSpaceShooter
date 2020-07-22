#pragma once

#include <functional>
#include <optional>

#include <glm/vec2.hpp>

enum class AnimationLayout { Horizontal, Vertical };
enum class AnimationPlayback { Forward, Backward };
enum class AnimationPolicy { StopOnLastFrame, Repeat, Pingpong };

namespace Tools
{
	struct TextureAnimationController
	{
	public:
		struct FrameTransformation
		{
			glm::vec2 translate;
			glm::vec2 scale;
		};

		TextureAnimationController(glm::ivec2 imageSize, glm::ivec2 startPosition, glm::ivec2 frameSize, glm::ivec2 framesGrid, glm::vec2 frameStep,
			float frameTime, int numOfFrames = -1, AnimationLayout animationLayout = AnimationLayout::Horizontal,
			AnimationPlayback animationPlayback = AnimationPlayback::Forward, AnimationPolicy animationPolicy = AnimationPolicy::Repeat,
			glm::vec2 scale = { 1.0f, 1.0f });

		FrameTransformation getFrameTransformation() const;

		void start();
		void stop();
		void pause();
		void resume();

	private:
		glm::ivec2 imageSize;
		glm::ivec2 startPosition;
		glm::ivec2 frameSize;
		glm::ivec2 framesGrid;
		glm::vec2 frameStep;
		float frameTime;
		int numOfFrames;
		AnimationLayout animationLayout;
		AnimationPlayback animationPlayback;
		AnimationPolicy animationPolicy;
		glm::vec2 scale;
		glm::vec2 textureScale;
		glm::vec2 frameScale;

		std::optional<float> startTime;
		std::optional<float> pauseTime;
	};
}
