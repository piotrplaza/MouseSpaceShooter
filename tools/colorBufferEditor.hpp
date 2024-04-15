#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <algorithm>

namespace Tools
{
	template <typename ColorType, bool doubleBuffering = false>
	class ColorBufferEditor
	{
	public:
		enum class Bottom { Down, Left, Up, Right };
		ColorBufferEditor(std::vector<ColorType>& colorBuffer, glm::ivec2 res, Bottom bottom = Bottom::Down) :
			colorBuffer(colorBuffer),
			res(res),
			bottom(bottom),
			border(0.0f)
		{
			updateRes(bottom);
			if constexpr (doubleBuffering)
				backColorBuffer = colorBuffer;
		}

		void clear(ColorType color = ColorType(0.0f))
		{
			std::fill(colorBuffer.begin(), colorBuffer.end(), color);
		}

		void putColor(const glm::ivec2& pos, const ColorType& color)
		{
			assert(pos.x >= 0 && pos.x < res.x);
			assert(pos.y >= 0 && pos.y < res.y);

			bufferTransformedLocation(pos, colorBuffer) = color;
		}

		ColorType getColor(const glm::ivec2& pos) const
		{
			if (pos.x < 0 || pos.x >= res.x || pos.y < 0 || pos.y >= res.y)
				return border;

			if constexpr (doubleBuffering)
				return bufferTransformedLocation(pos, backColorBuffer);
			else
				return bufferTransformedLocation(pos, colorBuffer);
		}

		glm::ivec2 getRes() const
		{
			return res;
		}

		void setBottom(Bottom bottom)
		{
			updateRes(bottom);
			this->bottom = bottom;
		}

		Bottom getBottom() const
		{
			return bottom;
		}

		void setBorder(ColorType color)
		{
			border = color;
		}

		ColorType getBorder() const
		{
			return border;
		}

		void swapBuffers()
		{
			static_assert(doubleBuffering);

			std::swap(colorBuffer, backColorBuffer);
		}

		std::vector<ColorType>& getColorBuffer()
		{
			return colorBuffer;
		}

		const std::vector<ColorType>& getColorBuffer() const
		{
			return colorBuffer;
		}

		std::vector<ColorType>& getBackColorBuffer()
		{
			static_assert(doubleBuffering);

			return backColorBuffer;
		}

		const std::vector<ColorType>& getBackColorBuffer() const
		{
			static_assert(doubleBuffering);

			return backColorBuffer;
		}

		constexpr static bool IsDoubleBuffering()
		{
			return doubleBuffering;
		}

	private:
		void updateRes(Bottom bottom)
		{
			if (((this->bottom == Bottom::Down || this->bottom == Bottom::Up) && (bottom == Bottom::Left || bottom == Bottom::Right))
				|| ((this->bottom == Bottom::Left || this->bottom == Bottom::Right) && (bottom == Bottom::Down || bottom == Bottom::Up)))
			{
				res = { res.y, res.x };
			}
		}

		ColorType& bufferTransformedLocation(const glm::ivec2& pos, std::vector<ColorType>& buffer)
		{
			switch (bottom)
			{
			case Bottom::Down:
				return buffer[pos.y * res.x + pos.x];
			case Bottom::Left:
				return buffer[(res.x - 1 - pos.x) * res.y + pos.y];
			case Bottom::Up:
				return buffer[(res.y - 1 - pos.y) * res.x + (res.x - 1 - pos.x)];
			case Bottom::Right:
				return buffer[pos.x * res.y + (res.y - 1 - pos.y)];
			}

			assert(!"unsupported bottom");
			return colorBuffer[0];
		}

		const ColorType& bufferTransformedLocation(const glm::ivec2& pos, const std::vector<ColorType>& buffer) const
		{
			return const_cast<ColorBufferEditor*>(this)->bufferTransformedLocation(pos, const_cast<std::vector<ColorType>&>(buffer));
		}

		glm::ivec2 res;
		Bottom bottom;
		std::vector<ColorType>& colorBuffer;
		std::vector<ColorType> backColorBuffer;
		ColorType border;
	};
}
