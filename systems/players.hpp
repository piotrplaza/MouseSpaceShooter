#pragma once

#include <memory>
#include <functional>
#include <optional>
#include <vector>
#include <unordered_map>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <ogl/shaders.hpp>
#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/shaders/colored.hpp>
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

		void step();
		void render() const;

	private:
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

		void turn(glm::vec2 controllerDelta) const;
		void throttle(bool active) const;
		void magneticHook(bool active) const;
		void createGrappleJoint() const;

		void updatePlayersPositionsBuffers();
		void updatePlayersTexCoordBuffers();
		void updateConnectionsGraphicsBuffers();

		void basicRender() const;
		void coloredRender() const;
		void sceneCoordTexturedRender() const;

		Components::Player& player; //Temporary.

		std::unique_ptr<Shaders::Programs::Basic> basicShadersProgram;
		std::unique_ptr<Shaders::Programs::Textured> texturedShadersProgram;
		std::unique_ptr<Shaders::Programs::Colored> coloredShadersProgram;

		std::unique_ptr<Buffers::PosTexCoordBuffers> simplePlayersBuffers;
		std::unordered_map<unsigned, Buffers::PosTexCoordBuffers> texturesToPlayersBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customSimplePlayersBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customTexturedPlayersBuffers;

		std::unique_ptr<ConnectionsBuffers> connectionsBuffers;
	};
}
