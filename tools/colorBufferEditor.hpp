#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <algorithm>

namespace Tools
{
	template <typename ColorType>
	class ColorBufferEditor
	{
	public:
		enum class Bottom { Down, Left, Up, Right };
		ColorBufferEditor(std::vector<ColorType>& colorBuffer, glm::ivec2 res, Bottom bottom = Bottom::Down) :
			colorBuffer(colorBuffer),
			res(res),
			bottom(Bottom::Down)
		{
			updateRes(bottom);
		}

		void clear(ColorType color = ColorType(0.0f))
		{
			std::fill(colorBuffer.begin(), colorBuffer.end(), color);
		}

		void putColor(const glm::ivec2& pos, const ColorType& color)
		{
			assert(pos.x >= 0 && pos.x < res.x);
			assert(pos.y >= 0 && pos.y < res.y);

			colorBufferTransformedLocation(pos) = color;
		}

		glm::vec3 getColor(const glm::ivec2& pos) const
		{
			assert(pos.x >= 0 && pos.x < res.x);
			assert(pos.y >= 0 && pos.y < res.y);

			return colorBufferTransformedLocation(pos);
		}

		const glm::vec3* data() const
		{
			return colorBuffer.data();
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

	private:
		void updateRes(Bottom bottom)
		{
			if (((this->bottom == Bottom::Down || this->bottom == Bottom::Up) && (bottom == Bottom::Left || bottom == Bottom::Right))
				|| ((this->bottom == Bottom::Left || this->bottom == Bottom::Right) && (bottom == Bottom::Down || bottom == Bottom::Up)))
			{
				res = { res.y, res.x };
			}
		}

		glm::vec3& colorBufferTransformedLocation(const glm::ivec2& pos)
		{
			switch (bottom)
			{
			case Bottom::Down:
				return colorBuffer[pos.y * res.x + pos.x];
			case Bottom::Left:
				return colorBuffer[(res.x - 1 - pos.x) * res.y + pos.y];
			case Bottom::Up:
				return colorBuffer[(res.y - 1 - pos.y) * res.x + (res.x - 1 - pos.x)];
			case Bottom::Right:
				return colorBuffer[pos.x * res.y + (res.y - 1 - pos.y)];
			}

			assert(!"unsupported bottom");
			return colorBuffer[0];
		}

		const glm::vec3& colorBufferTransformedLocation(const glm::ivec2& pos) const
		{
			return const_cast<ColorBufferEditor*>(this)->colorBufferTransformedLocation(pos);
		}

		glm::ivec2 res;
		Bottom bottom;
		std::vector<ColorType>& colorBuffer;
	};
}
