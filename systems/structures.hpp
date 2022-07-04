#pragma once

#include <ogl/buffers/genericBuffers.hpp>

#include <components/typeComponentMappers.hpp>

#include <memory>
#include <vector>
#include <unordered_map>
#include <variant>

namespace Systems
{
	class Structures
	{
	public:
		Structures();

		void postInit();
		void step();
		void render() const;

		void updateStaticBuffers();

	private:
		void initGraphics();

		void updateDynamicBuffers();

		void customShadersRender() const;
		void texturedRender() const;
		void basicRender() const;

		template <typename BufferType>
		struct StructuresBuffers
		{
			BufferType simpleWalls;
			BufferType simpleGrapples;

			BufferType texturedWalls;
			BufferType texturedGrapples;

			BufferType customShadersWalls;
			BufferType customShadersGrapples;
		};

		StructuresBuffers<std::vector<Buffers::GenericBuffers>> staticBuffers;
		StructuresBuffers<std::unordered_map<ComponentId, Buffers::GenericBuffers>> dynamicBuffers;
	};
}
