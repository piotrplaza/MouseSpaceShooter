#pragma once

#include <vector>
#include <unordered_map>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <shaders.hpp>
#include <shaders/basic.hpp>
#include <shaders/sceneCoordTextured.hpp>

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
		void updateStaticWallsGraphics();

	private:
		void initGraphics();

		void updateDynamicWallsGraphics();
		void updateGrapplesGraphics();

		void updateWallsVerticesCache(std::vector<Components::Wall>& walls, std::vector<glm::vec3>& simpleWallsVerticesCache,
			std::unordered_map<unsigned, std::vector<glm::vec3>>& textureToWallsVerticesCache) const;

		Shaders::Programs::Basic basicShadersProgram;
		Shaders::Programs::SceneCoordTextured sceneCoordTextured;

		GLuint staticWallsVertexArray;
		GLuint staticWallsVertexBuffer;

		GLuint dynamicWallsVertexArray;
		GLuint dynamicWallsVertexBuffer;

		GLuint grapplesVertexArray;
		GLuint grapplesVertexBuffer;

		std::vector<glm::vec3> simpleStaticWallsVerticesCache;
		std::vector<glm::vec3> simpleDynamicWallsVerticesCache;
		std::unordered_map<unsigned, std::vector<glm::vec3>> textureToStaticWallsVerticesCache;
		std::unordered_map<unsigned, std::vector<glm::vec3>> textureToDynamicWallsVerticesCache;

		std::vector<glm::vec3> grapplesVerticesCache;
	};
}
