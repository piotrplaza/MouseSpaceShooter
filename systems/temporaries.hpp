#pragma once

#include <unordered_map>

#include <ogl/buffers/genericBuffers.hpp>

#include <components/componentId.hpp>

namespace Systems
{
	class Temporaries
	{
	public:
		Temporaries();

		void step();
		void render() const;

	private:
		void updateDynamicBuffers();

		void customShadersRender(const std::unordered_map<ComponentId, Buffers::GenericBuffers>& buffers) const;
		void texturedRender(const std::unordered_map<ComponentId, Buffers::GenericBuffers>& buffers) const;
		void basicRender(const std::unordered_map<ComponentId, Buffers::GenericBuffers>& buffer) const;

		std::unordered_map<ComponentId, Buffers::GenericBuffers> simpleRocketsBuffers;
		std::unordered_map<ComponentId, Buffers::GenericBuffers> texturedRocketsBuffers;
		std::unordered_map<ComponentId, Buffers::GenericBuffers> customShaderRocketsBuffers;
	};
}
