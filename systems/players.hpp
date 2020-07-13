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
#include <shaders/basic.hpp>
#include <shaders/sceneCoordTextured.hpp>
#include <shaders/colored.hpp>

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
		struct PlayersBuffers
		{
			PlayersBuffers();
			~PlayersBuffers();

			GLuint vertexArray;
			GLuint positionBuffer;

			std::optional<unsigned> texture;
			std::function<void(Shaders::ProgramId)> renderingSetup;

			std::vector<glm::vec3> positionsCache;
			size_t numOfAllocatedPositions = 0;
		};

		struct ConnectionsBuffers
		{
			ConnectionsBuffers();
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
		void updateConnectionsGraphicsBuffers();

		void basicRender() const;
		void coloredRender() const;
		void sceneCoordTexturedRender() const;

		Components::Player& player; //Temporary.

		std::unique_ptr<Shaders::Programs::Basic> basicShadersProgram;
		std::unique_ptr<Shaders::Programs::SceneCoordTextured> sceneCoordTexturedShadersProgram;
		std::unique_ptr<Shaders::Programs::Colored> coloredShadersProgram;

		std::unique_ptr<PlayersBuffers> simplePlayersBuffers;
		std::unordered_map<unsigned, PlayersBuffers> texturesToPlayersBuffers;
		std::vector<PlayersBuffers> customSimplePlayersBuffers;
		std::vector<PlayersBuffers> customTexturedPlayersBuffers;

		std::unique_ptr<ConnectionsBuffers> connectionsBuffers;

		bool firstStep = true;
		glm::vec2 playerPreviousPosition{ 0.0f, 0.0f };
	};
}
