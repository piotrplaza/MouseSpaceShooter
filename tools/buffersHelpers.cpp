#include "buffersHelpers.hpp"

#include <tools/utility.hpp>

#include <algorithm>
#include <execution>

namespace
{
	constexpr bool parallelProcessing = true;
}

namespace Tools
{
	SubImageData GetSubImage(
		const float* sourceImageData,
		const glm::ivec2& sourceImageSize,
		const glm::ivec2& offset,
		const glm::ivec2& subImageSize,
		int numOfChannels,
		std::vector<float>& operationalBuffer)
	{
		if (!sourceImageData || sourceImageSize.x <= 0 || sourceImageSize.y <= 0 || subImageSize.x <= 0 || subImageSize.y <= 0)
			return { nullptr, glm::ivec2(0), glm::ivec2(0) };

		const glm::ivec2 adjustedOffset = glm::clamp(offset, glm::ivec2(0), sourceImageSize - glm::ivec2(1));
		glm::ivec2 adjustedSubImageSize = subImageSize;

		if (adjustedOffset.x + subImageSize.x > sourceImageSize.x)
			adjustedSubImageSize.x = sourceImageSize.x - adjustedOffset.x;

		if (adjustedOffset.y + subImageSize.y > sourceImageSize.y)
			adjustedSubImageSize.y = sourceImageSize.y - adjustedOffset.y;

		if (adjustedSubImageSize.x <= 0 || adjustedSubImageSize.y <= 0)
			return { nullptr, glm::ivec2(0), glm::ivec2(0) };

		operationalBuffer.resize(adjustedSubImageSize.x * adjustedSubImageSize.y * numOfChannels);

		auto processRow = [&](const auto y_) {
			const int y = (int)y_;
			const int sourceIndex = ((adjustedOffset.y + y) * sourceImageSize.x + adjustedOffset.x) * numOfChannels;
			const int destIndex = y * adjustedSubImageSize.x * numOfChannels;

			std::memcpy(&operationalBuffer[destIndex], &sourceImageData[sourceIndex], adjustedSubImageSize.x * numOfChannels * sizeof(float));
		};

		if constexpr (parallelProcessing && 1)
		{
			Tools::ItToId itToId(0, adjustedSubImageSize.y);
			std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), processRow);
		}
		else
			for (int y = 0; y < adjustedSubImageSize.y; ++y)
				processRow(y);

		return { operationalBuffer.data(), adjustedSubImageSize , adjustedOffset };
	}

	SubImageData ClipSubImage(
		const float* subImageData,
		const glm::ivec2& subImageSize,
		const glm::ivec2& offset,
		const glm::ivec2& targetImageSize,
		const int numOfChannels,
		std::vector<float>& operationalBuffer)
	{
		const glm::ivec2 subImageEndInTargetSpace = offset + subImageSize;

		if (offset.x >= 0 && offset.y >= 0 && subImageEndInTargetSpace.x <= targetImageSize.x && subImageEndInTargetSpace.y <= targetImageSize.y)
			return { subImageData, subImageSize, offset };

		if (offset.x >= targetImageSize.x || offset.y >= targetImageSize.y || subImageEndInTargetSpace.x <= 0 || subImageEndInTargetSpace.y <= 0)
			return { nullptr, glm::ivec2(0), glm::ivec2(0) };

		const glm::ivec2 clippedStart(std::max(0, offset.x), std::max(0, offset.y));
		const glm::ivec2 clippedEnd(std::min(targetImageSize.x, subImageEndInTargetSpace.x), std::min(targetImageSize.y, subImageEndInTargetSpace.y));
		const glm::ivec2 clippedOffset = (clippedStart - offset);
		const glm::ivec2 clippedSize = clippedEnd - clippedStart;
		const int rowSize = clippedSize.x * numOfChannels * sizeof(float);

		operationalBuffer.resize(clippedSize.x * clippedSize.y * numOfChannels);

		auto processRow = [&](const auto y_) {
			const int y = (int)y_;
			const int sourceStartIndex = ((y + clippedOffset.y) * subImageSize.x + clippedOffset.x) * numOfChannels;
			const int destStartIndex = y * clippedSize.x * numOfChannels;
			std::memcpy(&operationalBuffer[destStartIndex], &subImageData[sourceStartIndex], rowSize);
		};

		if constexpr (parallelProcessing && 1)
		{
			Tools::ItToId itToId(clippedSize.y);
			std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), processRow);
		}
		else
			for (int y = 0; y < clippedSize.y; ++y)
				processRow(y);

		return { operationalBuffer.data(), clippedSize, clippedStart };
	}
}
