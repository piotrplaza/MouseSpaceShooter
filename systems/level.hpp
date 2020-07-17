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

#include <buffers/posTexCoordBuffers.hpp>

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
		void initGraphics();

		void updateStaticWallsPositionsBuffers();
		void updateDynamicWallsPositionsBuffers();
		void updateGrapplesPositionsBuffers();

		void updateTexCoordsBuffers();
		void updateDynamicWallsTexCoordsBuffers();
		void updateGrapplesTexCoordsBuffers();

		void sceneCoordTexturedRender() const;
		void texturedRender() const;
		void basicRender() const;

		std::unique_ptr<Shaders::Programs::Basic> basicShadersProgram;
		std::unique_ptr<Shaders::Programs::SceneCoordTextured> sceneCoordTexturedShadersProgram;
		std::unique_ptr<Shaders::Programs::Textured> texturedShadersProgram;

		std::unique_ptr<Buffers::PosTexCoordBuffers> simpleStaticWallsBuffers;
		std::unique_ptr<Buffers::PosTexCoordBuffers> simpleDynamicWallsBuffers;
		std::unordered_map<unsigned, Buffers::PosTexCoordBuffers> texturesToStaticWallsBuffers;
		std::unordered_map<unsigned, Buffers::PosTexCoordBuffers> texturesToDynamicWallsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customSimpleStaticWallsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customSimpleDynamicWallsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customTexturedStaticWallsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customTexturedDynamicWallsBuffers;

		std::unique_ptr<Buffers::PosTexCoordBuffers> simpleGrapplesBuffers;
		std::unordered_map<unsigned, Buffers::PosTexCoordBuffers> texturesToGrapplesBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customSimpleGrapplesBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customTexturedGrapplesBuffers;
	};
}
