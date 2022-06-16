#pragma once

#include <ogl/buffers/genericBuffers.hpp>

#include <components/componentId.hpp>

#include <memory>
#include <vector>
#include <unordered_map>

namespace Systems
{
	class Decorations
	{
	public:
		Decorations();

		void postInit();
		void step();

		void renderBackground() const;
		void renderFarMidground() const;
		void renderMidground() const;
		void renderNearMidground() const;
		void renderForeground() const;

	private:
		void initGraphics();

		void updatePersistentPositionsBuffers();
		void updatePersistentTexCoordsBuffers();

		void updateTemporaryPosAndTexCoordBuffers();

		void customShadersRender(const std::vector<Buffers::GenericBuffers>& persistentBuffers,
			const std::unordered_map<ComponentId, Buffers::GenericBuffers>& temporaryBuffers) const;
		void texturedRender(const std::vector<Buffers::GenericBuffers>& persistentBuffers,
			const std::unordered_map<ComponentId, Buffers::GenericBuffers>& temporaryBuffers) const;
		void basicRender(const std::vector<Buffers::GenericBuffers>& persistentBuffers,
			const std::unordered_map<ComponentId, Buffers::GenericBuffers>& temporaryBuffers) const;

		template <typename BufferType>
		struct DecorationBuffers
		{
			BufferType simpleBackgroundDecorations;
			BufferType simpleFarMidgroundDecorations;
			BufferType simpleMidgroundDecorations;
			BufferType simpleNearMidgroundDecorations;
			BufferType simpleForegroundDecorations;

			BufferType texturedBackgroundDecorations;
			BufferType texturedFarMidgroundDecorations;
			BufferType texturedMidgroundDecorations;
			BufferType texturedNearMidgroundDecorations;
			BufferType texturedForegroundDecorations;

			BufferType customShadersBackgroundDecorations;
			BufferType customShadersFarMidgroundDecorations;
			BufferType customShadersMidgroundDecorations;
			BufferType customShadersNearMidgroundDecorations;
			BufferType customShadersForegroundDecorations;
		};
		
		DecorationBuffers<std::vector<Buffers::GenericBuffers>> persistentBuffers;
		DecorationBuffers<std::unordered_map<ComponentId, Buffers::GenericBuffers>> temporaryBuffers;
	};
}
