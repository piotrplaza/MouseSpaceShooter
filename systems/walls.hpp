#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <variant>

#include <ogl/buffers/posTexCoordBuffers.hpp>

#include <commonTypes/blendingTexture.hpp>

namespace Components
{
	struct Wall;
}

namespace Systems
{
	class Walls
	{
	public:
		Walls();

		void postInit();
		void step();
		void render() const;

	private:
		void initGraphics();

		void updateStaticWallsPositionsBuffers();
		void updateDynamicWallsPositionsBuffers();
		void updateGrapplesPositionsBuffers();

		void updatePersistentTexCoordsBuffers();
		void updateDynamicWallsTexCoordsBuffers();
		void updateGrapplesTexCoordsBuffers();

		void customShadersRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const;
		void customShadersRender() const;
		void sceneCoordTexturedRender() const;
		void texturedRender() const;
		void basicRender() const;

		std::unique_ptr<Buffers::PosTexCoordBuffers> simpleStaticWallsBuffers;
		std::unique_ptr<Buffers::PosTexCoordBuffers> simpleDynamicWallsBuffers;
		std::unordered_map<std::variant<std::monostate, unsigned, BlendingTexture>, Buffers::PosTexCoordBuffers> texturesToStaticWallsBuffers;
		std::unordered_map<std::variant<std::monostate, unsigned, BlendingTexture>, Buffers::PosTexCoordBuffers> texturesToDynamicWallsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customSimpleStaticWallsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customSimpleDynamicWallsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customTexturedStaticWallsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customTexturedDynamicWallsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customShadersStaticWallsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customShadersDynamicWallsBuffers;

		std::unique_ptr<Buffers::PosTexCoordBuffers> simpleGrapplesBuffers;
		std::unordered_map<std::variant<std::monostate, unsigned, BlendingTexture>, Buffers::PosTexCoordBuffers> texturesToGrapplesBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customSimpleGrapplesBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customTexturedGrapplesBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customShadersGrapplesBuffers;
	};
}
