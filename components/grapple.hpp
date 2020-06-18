#include <vector>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <globals.hpp>

namespace Components
{
	struct Grapple
	{
		Grapple(const glm::vec2& position) : position(position)
		{
		}

		glm::vec2 position;
		std::vector<glm::vec3> verticesCache;

		void updateVerticesCache()
		{
			using namespace Globals::Defaults;

			verticesCache.clear();
			verticesCache.reserve(grappleGraphicsComplexity * 3);

			constexpr float radialStep = glm::two_pi<float>() / grappleGraphicsComplexity;

			for (int i = 0; i < grappleGraphicsComplexity; ++i)
			{
				const float radialPosition = i * radialStep;
				const float nextRadialPosition = (i + 1) * radialStep;

				verticesCache.emplace_back(position, 0.0f);
				verticesCache.emplace_back(position + glm::vec2(glm::cos(radialPosition), glm::sin(radialPosition)) * grappleRadius, 0.0f);
				verticesCache.emplace_back(position + glm::vec2(glm::cos(nextRadialPosition), glm::sin(nextRadialPosition)) * grappleRadius, 0.0f);
			}
		}
	};
}
