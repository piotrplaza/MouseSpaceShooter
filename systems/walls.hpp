#pragma once

#include <ogl/buffers/genericBuffers.hpp>

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

		void updateStaticBuffers();

		void customShadersRender(const std::vector<Buffers::GenericBuffers>& buffers) const;
		void customShadersRender() const;
		void texturedRender() const;
		void basicRender() const;

		std::vector<Buffers::GenericBuffers> simpleWallsBuffers;
		std::vector<Buffers::GenericBuffers> texturedWallsBuffers;
		std::vector<Buffers::GenericBuffers> customShadersWallsBuffers;

		std::vector<Buffers::GenericBuffers> simpleGrapplesBuffers;
		std::vector<Buffers::GenericBuffers> texturedGrapplesBuffers;
		std::vector<Buffers::GenericBuffers> customShadersGrapplesBuffers;
	};
}
