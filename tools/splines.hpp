#pragma once

#include <glm/vec2.hpp>

#include <vector>
#include <cmath>

namespace Tools
{
	template<typename Vec = glm::vec2>
	class CubicHermiteSpline
	{
	public:
		CubicHermiteSpline(std::vector<Vec> keypoints, bool preBeginIncluded = false, bool postEndIncluded = false)
		{
			assert(keypoints.size() + !preBeginIncluded + !postEndIncluded >= 4);

			if (preBeginIncluded && postEndIncluded)
			{
				this->keypoints = std::move(keypoints);
				return;
			}

			this->keypoints.reserve(keypoints.size() + !preBeginIncluded + !postEndIncluded);

			if (!preBeginIncluded)
				this->keypoints.push_back(keypoints[0] + keypoints[0] - keypoints[1]);

			this->keypoints.insert(this->keypoints.end(), keypoints.begin(), keypoints.end());

			if (!postEndIncluded)
				this->keypoints.push_back(keypoints.back() + keypoints.back() - keypoints[keypoints.size() - 2]);
		}

		Vec getInterpolation(float t)
		{
			assert(t >= 0.0f && t <= 1.0f);

			if (t == 1.0f)
				return keypoints[keypoints.size() - 2];

			const float realIndex = (keypoints.size() - 3) * t + 1;
			const size_t i0 = (size_t)std::floor(realIndex) - 1;
			const size_t i1 = i0 + 1;
			const size_t i2 = i1 + 1;
			const size_t i3 = i2 + 1;
			const float localT = std::fmod((keypoints.size() - 3.0f) * t, 1.0f);

			return cubicHermite(keypoints[i0], keypoints[i1], keypoints[i2], keypoints[i3], localT);
		}

		Vec getPostprocessedInterpolation(float t, std::function<Vec(Vec)> postprocessF)
		{
			assert(t >= 0.0f && t <= 1.0f);

			return postprocessF(getInterpolation(t));
		}

		std::vector<Vec>& accessKeypoints()
		{
			return keypoints;
		}

		const std::vector<Vec>& getKeypoints() const
		{
			return keypoints;
		}

	private:
		Vec cubicHermite(Vec v0, Vec v1, Vec v2, Vec v3, float t) const
		{
			const Vec a = -v0 / 2.0f + (3.0f * v1) / 2.0f - (3.0f * v2) / 2.0f + v3 / 2.0f;
			const Vec b = v0 - (5.0f * v1) / 2.0f + 2.0f * v2 - v3 / 2.0f;
			const Vec c = -v0 / 2.0f + v2 / 2.0f;
			const Vec d = v1;

			return a * t * t * t + b * t * t + c * t + d;
		}

		std::vector<Vec> keypoints;
	};
}
