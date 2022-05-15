#pragma once

#include <ogl/buffers/posTexCoordBuffers.hpp>

#include <components/typeComponentMappers.hpp>

#include <memory>
#include <vector>
#include <unordered_map>
#include <variant>

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

		void updateWallsPositionsBuffers();
		void updateGrapplesPositionsBuffers();

		void updateTexCoordsBuffers();
		void updateWallsTexCoordsBuffers();
		void updateGrapplesTexCoordsBuffers();

		void customShadersRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const;
		void customShadersRender() const;
		void texturedRender() const;
		void basicRender() const;

		std::vector<Buffers::PosTexCoordBuffers> simpleWallsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> texturedWallsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customShadersWallsBuffers;

		std::vector<Buffers::PosTexCoordBuffers> simpleGrapplesBuffers;
		std::vector<Buffers::PosTexCoordBuffers> texturedGrapplesBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customShadersGrapplesBuffers;
	};
}
