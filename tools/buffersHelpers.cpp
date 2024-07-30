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
		const glm::ivec2& offsetPos,
		const glm::ivec2& subImageSize,
		int numOfChannels,
		std::vector<float>& operationalBuffer)
	{
		if (!sourceImageData || sourceImageSize.x <= 0 || sourceImageSize.y <= 0 || subImageSize.x <= 0 || subImageSize.y <= 0)
			return { nullptr, glm::ivec2(0), glm::ivec2(0) };

		const glm::ivec2 adjustedOffsetPos = glm::clamp(offsetPos, glm::ivec2(0), sourceImageSize - glm::ivec2(1));
		glm::ivec2 adjustedSubImageSize = subImageSize;

		if (adjustedOffsetPos.x + subImageSize.x > sourceImageSize.x)
			adjustedSubImageSize.x = sourceImageSize.x - adjustedOffsetPos.x;

		if (adjustedOffsetPos.y + subImageSize.y > sourceImageSize.y)
			adjustedSubImageSize.y = sourceImageSize.y - adjustedOffsetPos.y;

		if (adjustedSubImageSize.x <= 0 || adjustedSubImageSize.y <= 0)
			return { nullptr, glm::ivec2(0), glm::ivec2(0) };

		operationalBuffer.resize(adjustedSubImageSize.x * adjustedSubImageSize.y * numOfChannels);

		auto processRow = [&](const auto y) {
			const int sourceIndex = ((adjustedOffsetPos.y + y) * sourceImageSize.x + adjustedOffsetPos.x) * numOfChannels;
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

		return { operationalBuffer.data(), adjustedSubImageSize , adjustedOffsetPos };
	}

	SubImageData GetClippedSubImage(
		float* subImageData,
		const glm::ivec2& subImageSize,
		const glm::ivec2& offsetPos,
		const glm::ivec2& targetImageSize,
		const int numOfChannels,
		std::vector<float>& operationalBuffer)
	{
		const int startX = offsetPos.x;
		const int startY = offsetPos.y;
		const int endX = startX + subImageSize.x;
		const int endY = startY + subImageSize.y;

		const bool withinBounds = (startX >= 0) && (startY >= 0) && (endX <= targetImageSize.x) && (endY <= targetImageSize.y);

		if (withinBounds)
			return { subImageData, subImageSize, offsetPos };
		else
		{
			if (startX >= targetImageSize.x || startY >= targetImageSize.y || endX <= 0 || endY <= 0)
				return { nullptr, glm::ivec2(0), glm::ivec2(0) };

			const int clippedStartX = std::max(0, startX);
			const int clippedStartY = std::max(0, startY);
			const int clippedEndX = std::min(targetImageSize.x, endX);
			const int clippedEndY = std::min(targetImageSize.y, endY);

			const glm::ivec2 clippedSize(clippedEndX - clippedStartX, clippedEndY - clippedStartY);
			const glm::ivec2 newOffset(std::max(0, startX), std::max(0, startY));

			const int rowSize = clippedSize.x * numOfChannels * sizeof(float);
			const int sourceX = clippedStartX - startX;

			operationalBuffer.resize(clippedSize.x * clippedSize.y * numOfChannels);

			auto processRow = [&](const auto y) {
				const int sourceY = y - startY;
				const int destIndex = (y - clippedStartY) * clippedSize.x * numOfChannels;
				const int sourceIndex = (sourceY * subImageSize.x + sourceX) * numOfChannels;

				std::memcpy(&operationalBuffer[destIndex], &subImageData[sourceIndex], rowSize);
			};

			if constexpr (parallelProcessing && 1)
			{
				Tools::ItToId itToId(clippedStartY, clippedEndY);
				std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), processRow);
			}
			else
				for (int y = clippedStartY; y < clippedEndY; ++y)
					processRow(y);

			return { operationalBuffer.data(), clippedSize, newOffset };
		}
	}
}
