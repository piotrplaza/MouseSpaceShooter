#pragma once

#include <commonTypes/componentId.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>
#include <unordered_map>

namespace Components
{
	struct Plane;
}

namespace Systems
{
	class Actors
	{
	public:
		Actors();

		void postInit();
		void step();

		void updateDynamicBuffers();

	private:
		struct Connections
		{
			struct Params
			{
				Params(const glm::vec2& p1, const glm::vec2& p2, const glm::vec4& color, int segmentsNum = 1, float frayFactor = 0.5f)
					: p1(p1), p2(p2), color(color), segmentsNum(segmentsNum), frayFactor(frayFactor)
				{
				}

				glm::vec2 p1;
				glm::vec2 p2;

				glm::vec4 color;
				int segmentsNum;
				float frayFactor;

				std::vector<glm::vec3> getVertices() const;
				std::vector<glm::vec4> getColors() const;
			};

			Connections();
			~Connections();

			std::vector<Params> params;
			ComponentId decorationId;

			void updateBuffers();
		};

		void turn(Components::Plane& plane) const;
		void throttle(Components::Plane& plane) const;
		void magneticHook(Components::Plane& plane, Connections& planeConnections);
		void createGrappleJoint(Components::Plane& plane) const;

		std::unordered_map<ComponentId, Connections> allConnections;
	};
}
