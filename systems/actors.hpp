#pragma once

#include <ogl/buffers/genericBuffers.hpp>

#include <components/typeComponentMappers.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <memory>
#include <functional>
#include <optional>
#include <vector>
#include <unordered_map>
#include <variant>

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
		void render() const;

		void updateStaticBuffers();

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

			std::vector<Params> params;
			std::vector<glm::vec3> vertices;
			std::vector<glm::vec4> colors;
			Buffers::GenericBuffers buffers;

			void updateBuffers();
		};

		void initGraphics();

		void turn(Components::Plane& plane) const;
		void throttle(Components::Plane& plane) const;
		void magneticHook(Components::Plane& plane);
		void createGrappleJoint(Components::Plane& plane) const;

		void basicRender() const;
		void texturedRender() const;
		void customShadersRender() const;
		void coloredRender() const;

		std::vector<Buffers::GenericBuffers> simplePlaneBuffers;
		std::vector<Buffers::GenericBuffers> texturedPlaneBuffers;
		std::vector<Buffers::GenericBuffers> customShadersPlaneBuffers;

		Connections connections;
	};
}
