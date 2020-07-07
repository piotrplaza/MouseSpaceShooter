#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <shaders.hpp>
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
		void updateStaticWallsGraphics();

	private:
		struct WallsBuffers
		{
			WallsBuffers();
			~WallsBuffers();

			GLuint vertexArray;
			GLuint vertexBuffer;

			std::vector<glm::vec3> verticesCache;
			size_t vertexBufferAllocation = 0;
		};

		struct GrapplesBuffers
		{
			GrapplesBuffers();
			~GrapplesBuffers();

			GLuint vertexArray;
			GLuint vertexBuffer;

			std::vector<glm::vec3> verticesCache;
			size_t vertexBufferAllocation = 0;
		};

		void initGraphics();

		void updateDynamicWallsGraphics();
		void updateGrapplesGraphics();

		void updateWallsBuffers(std::vector<Components::Wall>& walls, WallsBuffers& simpleWallsBuffers,
			std::unordered_map<unsigned, WallsBuffers>& texturesToWallsBuffers, GLenum bufferDataUsage) const;

		std::unique_ptr<Shaders::Programs::Basic> basicShadersProgram;
		std::unique_ptr<Shaders::Programs::SceneCoordTextured> sceneCoordTexturedShadersProgram;
		std::unique_ptr<Shaders::Programs::Textured> texturedShadersProgram;

		std::unique_ptr<WallsBuffers> staticWallsBuffers;
		std::unique_ptr<WallsBuffers> dynamicWallsBuffers;
		std::unordered_map<unsigned, WallsBuffers> texturesToStaticWallsBuffers;
		std::unordered_map<unsigned, WallsBuffers> texturesToDynamicWallsBuffers;

		std::unique_ptr<GrapplesBuffers> grapplesBuffers;
	};
}
