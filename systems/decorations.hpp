#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/buffers/posTexCoordBuffers.hpp>

#include <componentId.hpp>

namespace Systems
{
	class Decorations
	{
	public:
		Decorations();

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

		void customShadersRender(const std::vector<Buffers::PosTexCoordBuffers>& persistentBuffers,
			const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& temporaryBuffers) const;
		void texturedRender(const std::vector<Buffers::PosTexCoordBuffers>& persistentBuffers,
			const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& temporaryBuffers) const;
		void basicRender(const std::vector<Buffers::PosTexCoordBuffers>& persistentBuffers,
			const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& temporaryBuffers) const;

		void renderTexturesFramebuffers(bool lowerLinear, bool lowestLinear, bool pixelArt) const;

		std::unique_ptr<Shaders::Programs::Basic> basicShadersProgram;
		std::unique_ptr<Shaders::Programs::Textured> texturedShadersProgram;

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
		
		DecorationBuffers<std::vector<Buffers::PosTexCoordBuffers>> persistentBuffers;
		DecorationBuffers<std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>> temporaryBuffers;
	};
}
