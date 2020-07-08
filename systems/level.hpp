#pragma once

#include <memory>
#include <vector>
#include <optional>
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
		struct WallsBuffers
		{
			WallsBuffers(bool texCoord = false);
			~WallsBuffers();

			GLuint vertexArray;
			GLuint vertexBuffer;
			std::optional<GLuint> texCoordBuffer;

			std::vector<glm::vec3> verticesCache;
			std::vector<glm::vec2> texCoordCache;
			size_t vertexBufferAllocation = 0;
		};

		struct GrapplesBuffers
		{
			GrapplesBuffers(bool texCoord = false);
			~GrapplesBuffers();

			GLuint vertexArray;
			GLuint vertexBuffer;
			std::optional<GLuint> texCoordBuffer;

			std::vector<glm::vec3> verticesCache;
			std::vector<glm::vec2> texCoordCache;
			size_t vertexBufferAllocation = 0;
		};

		void initGraphics();

		void updateStaticWallsGraphics();
		void updateDynamicWallsGraphics();
		void updateTexCoords();
		void updateGrapplesGraphics();

		void updateWallsBuffers(std::vector<Components::Wall>& walls, WallsBuffers& simpleWallsBuffers,
			std::unordered_map<unsigned, WallsBuffers>& texturesToWallsBuffers, GLenum bufferDataUsage) const;

		void sceneCoordTexturedRender() const;
		void texturedRender() const;
		void basicRender() const;

		std::unique_ptr<Shaders::Programs::Basic> basicShadersProgram;
		std::unique_ptr<Shaders::Programs::SceneCoordTextured> sceneCoordTexturedShadersProgram;
		std::unique_ptr<Shaders::Programs::Textured> texturedShadersProgram;

		std::unique_ptr<WallsBuffers> simpleStaticWallsBuffers;
		std::unique_ptr<WallsBuffers> simpleDynamicWallsBuffers;
		std::unordered_map<unsigned, WallsBuffers> texturesToStaticWallsBuffers;
		std::unordered_map<unsigned, WallsBuffers> texturesToDynamicWallsBuffers;

		std::unique_ptr<GrapplesBuffers> simpleGrapplesBuffers;
		std::unordered_map<unsigned, GrapplesBuffers> texturesToGrapplesBuffers;
	};
}
