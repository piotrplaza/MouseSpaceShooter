#pragma once

#include <memory>
#include <functional>
#include <optional>
#include <vector>
#include <unordered_map>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <ogl/buffers/posTexCoordBuffers.hpp>

namespace Components
{
	struct Player;
}

namespace Systems
{
	class Players
	{
	public:
		Players();

		void postInit();
		void step();
		void render() const;

	private:
		struct Connection
		{
			Connection(const glm::vec2& p1, const glm::vec2& p2, const glm::vec4& color, int segmentsNum = 1, float frayFactor = 0.5f)
				: p1(p1), p2(p2), color(color), segmentsNum(segmentsNum), frayFactor(frayFactor)
			{
			}

			glm::vec2 p1;
			glm::vec2 p2;

			glm::vec4 color;
			int segmentsNum;
			float frayFactor;

			std::vector<glm::vec3> getPositions() const;
			std::vector<glm::vec4> getColors() const;
		};

		struct ConnectionsBuffers
		{
			ConnectionsBuffers();
			ConnectionsBuffers(const ConnectionsBuffers&) = delete;

			~ConnectionsBuffers();

			GLuint vertexArray;
			GLuint positionBuffer;
			GLuint colorBuffer;

			std::vector<glm::vec3> positionsCache;
			std::vector<glm::vec4> colorsCache;
			size_t numOfAllocatedVertices = 0;
		};

		void initGraphics();

		void turn(Components::Player& player, glm::vec2 controllerDelta) const;
		void throttle(Components::Player& player, bool active) const;
		void magneticHook(Components::Player& player, bool active);
		void createGrappleJoint(Components::Player& player) const;

		void updatePlayersPositionsBuffers();
		void updatePlayersTexCoordBuffers();
		void updateConnectionsGraphicsBuffers();

		void basicRender() const;
		void sceneCoordTexturedRender() const;
		void customShadersRender() const;
		void coloredRender() const;

		std::unique_ptr<Buffers::PosTexCoordBuffers> simplePlayersBuffers;
		std::unordered_map<unsigned, Buffers::PosTexCoordBuffers> texturesToPlayersBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customSimplePlayersBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customTexturedPlayersBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customShadersPlayersBuffers;

		std::vector<Connection> connections;
		std::unique_ptr<ConnectionsBuffers> connectionsBuffers;
	};
}
