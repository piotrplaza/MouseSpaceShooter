#pragma once

#include "_componentBase.hpp"
#include "details/animationData.hpp"

namespace Components
{
	struct AnimatedTexture : ComponentBase, AnimationData
	{
		AnimatedTexture() = default;

		AnimatedTexture(CM::Texture texture, glm::ivec2 textureSize, glm::ivec2 framesGrid, glm::ivec2 leftTopFrameLeftTopCorner, int rightTopFrameLeftEdge, int leftBottomFrameTopEdge,
			glm::ivec2 frameSize, float frameDuration, int numOfFrames = 0, int startFrame = 0, Direction animationDirection = AnimationData::Direction::Forward,
			Mode animationMode = AnimationData::Mode::Repeat, TextureLayout textureLayout = AnimationData::TextureLayout::Horizontal) :
			AnimationData(texture, textureSize, framesGrid, leftTopFrameLeftTopCorner, rightTopFrameLeftEdge, leftBottomFrameTopEdge, frameSize, frameDuration, numOfFrames, startFrame, animationDirection, animationMode, textureLayout)
		{}

		void setAnimationData(const AnimationData& animationData)
		{
			static_cast<AnimationData&>(*this) = animationData;
		}
	};
}
