#pragma once

#include <memory>
#include <optional>
#include <functional>
#include <vector>
#include <unordered_map>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <ogl/shaders.hpp>
#include <shaders/basic.hpp>
#include <shaders/sceneCoordTextured.hpp>
#include <shaders/textured.hpp>

namespace Components
{
	struct Wall;
}

namespace Systems
{
	class Level
	{
	public:
		Level();

		void step();
		void render() const;

	private:
		struct PosTexCoordBuffers
		{
			PosTexCoordBuffers();
			~PosTexCoordBuffers();

			void createTexCoordBuffer();

			GLuint vertexArray;
			GLuint positionBuffer;

			std::optional<GLuint> texCoordBuffer;
			std::optional<unsigned> texture;
			std::function<void(Shaders::ProgramId)> renderingSetup;

			std::vector<glm::vec3> positionsCache;
			std::vector<glm::vec2> texCoordCache;
			size_t numOfAllocatedPositions = 0;
			size_t numOfAllocatedTexCoord = 0;
		};

		void initGraphics();

		void updateTexCoordsBuffers();
		void updateDynamicWallsTexCoordsBuffers();
		void updateGrapplesTexCoordsBuffers();
		void updateStaticWallsPositionsBuffers();
		void updateDynamicWallsPositionsBuffers();
		void updateGrapplesPositionsBuffers();

		void sceneCoordTexturedRender() const;
		void texturedRender() const;
		void basicRender() const;

		std::unique_ptr<Shaders::Programs::Basic> basicShadersProgram;
		std::unique_ptr<Shaders::Programs::SceneCoordTextured> sceneCoordTexturedShadersProgram;
		std::unique_ptr<Shaders::Programs::Textured> texturedShadersProgram;

		std::unique_ptr<PosTexCoordBuffers> simpleStaticWallsBuffers;
		std::unique_ptr<PosTexCoordBuffers> simpleDynamicWallsBuffers;
		std::unordered_map<unsigned, PosTexCoordBuffers> texturesToStaticWallsBuffers;
		std::unordered_map<unsigned, PosTexCoordBuffers> texturesToDynamicWallsBuffers;
		std::vector<PosTexCoordBuffers> customSimpleStaticWallsBuffers;
		std::vector<PosTexCoordBuffers> customSimpleDynamicWallsBuffers;
		std::vector<PosTexCoordBuffers> customTexturedStaticWallsBuffers;
		std::vector<PosTexCoordBuffers> customTexturedDynamicWallsBuffers;

		std::unique_ptr<PosTexCoordBuffers> simpleGrapplesBuffers;
		std::unordered_map<unsigned, PosTexCoordBuffers> texturesToGrapplesBuffers;
		std::vector<PosTexCoordBuffers> customSimpleGrapplesBuffers;
		std::vector<PosTexCoordBuffers> customTexturedGrapplesBuffers;
	};
}
