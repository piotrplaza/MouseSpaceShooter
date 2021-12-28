#pragma once

#include <functional>
#include <optional>

#include <glm/vec2.hpp>

#include <componentBase.hpp>

enum class AnimationLayout { Horizontal, Vertical };
enum class AnimationPlayback { Forward, Backward };
enum class AnimationPolicy { StopOnLastFrame, Repeat, Pingpong };

namespace Components
{
	struct AnimationTexture : ComponentBase
	{
		using ComponentBase::ComponentBase;

		struct FrameTransformation
		{
			glm::vec2 translate;
			glm::vec2 scale;
		};

		AnimationTexture(unsigned textureId, glm::ivec2 imageSize, glm::ivec2 startPosition, glm::ivec2 frameSize, glm::ivec2 framesGrid, glm::vec2 frameStep,
			float frameDuration, int numOfFrames = 0, AnimationLayout animationLayout = AnimationLayout::Horizontal,
			AnimationPlayback animationPlayback = AnimationPlayback::Forward, AnimationPolicy animationPolicy = AnimationPolicy::Repeat,
			glm::vec2 translate = { 0.0f, 0.0f }, glm::vec2 scale = { 1.0f, 1.0f });

		unsigned getTextureId() const;

		FrameTransformation getFrameTransformation() const;

		void start();
		void stop();
		void pause();
		void resume();

		void setDurationScale(float durationScale);
		float getDurationScale() const;

	private:
		unsigned textureId;

		glm::ivec2 imageSize;
		glm::ivec2 startPosition;
		glm::ivec2 framesGrid;
		glm::vec2 frameStep;
		float frameDuration;
		int numOfFrames;
		AnimationLayout animationLayout;
		AnimationPlayback animationPlayback;
		AnimationPolicy animationPolicy;
		glm::vec2 translate;
		glm::vec2 scale;
		glm::vec2 textureScale;
		glm::vec2 frameScale;

		std::optional<float> pauseDuration;
		float durationScale = 1.0f;

		mutable std::optional<float> prevDuration;
		mutable float animationDuration = 0.0f;
	};
}
