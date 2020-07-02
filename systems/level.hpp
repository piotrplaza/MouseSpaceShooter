#pragma once

#include <vector>
#include <unordered_map>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "shaders.hpp"

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
		void renderBackground() const;
		void renderForeground() const;
		void updateStaticWallsGraphics();

	private:
		void initGraphics();

		void updateDynamicWallsGraphics();
		void updateGrapplesGraphics();
		void updateConnectionsGraphics();

		void updateWallsVerticesCache(std::vector<Components::Wall>& walls, std::vector<glm::vec3>& simpleWallsVerticesCache,
			std::unordered_map<unsigned, std::vector<glm::vec3>>& textureToWallsVerticesCache) const;

		shaders::ProgramId basicShadersProgram;
		GLint basicShadersMVPUniform;
		GLint basicShadersColorUniform;

		shaders::ProgramId coloredShadersProgram;
		GLint coloredShadersMVPUniform;

		shaders::ProgramId sceneCoordTexturedShadersProgram;
		GLint sceneCoordTexturedShadersMVPUniform;
		GLint sceneCoordTexturedShadersModelUniform;
		GLint sceneCoordTexturedShadersTexture1Uniform;
		GLint sceneCoordTexturedShadersTextureScalingUniform;

		GLuint staticWallsVertexArray;
		GLuint staticWallsVertexBuffer;

		GLuint dynamicWallsVertexArray;
		GLuint dynamicWallsVertexBuffer;

		GLuint grapplesVertexArray;
		GLuint grapplesVertexBuffer;

		GLuint connectionsVertexArray;
		GLuint connectionsVertexBuffer;
		GLuint connectionsColorBuffer;

		std::vector<glm::vec3> simpleStaticWallsVerticesCache;
		std::vector<glm::vec3> simpleDynamicWallsVerticesCache;
		std::unordered_map<unsigned, std::vector<glm::vec3>> textureToStaticWallsVerticesCache;
		std::unordered_map<unsigned, std::vector<glm::vec3>> textureToDynamicWallsVerticesCache;

		std::vector<glm::vec3> grapplesVerticesCache;

		std::vector<glm::vec3> connectionsVerticesCache;
		std::vector<glm::vec4> connectionsColorsCache;
	};
}
