#pragma once

#include <unordered_map>

#include <ogl/buffers/posTexCoordBuffers.hpp>

#include <componentId.hpp>

namespace Systems
{
	class Temporaries
	{
	public:
		Temporaries();

		void step();
		void render() const;

	private:
		void updatePosAndTexCoordBuffers();

		void customShadersRender(const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& buffers) const;
		void texturedRender(const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& buffers) const;
		void basicRender(const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& buffer) const;

		std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers> simpleRocketsBuffers;
		std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers> texturedRocketsBuffers;
		std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers> customShaderRocketsBuffers;
	};
}
