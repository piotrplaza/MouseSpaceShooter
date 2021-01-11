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
		void renderMidground() const;
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

		std::unique_ptr<Shaders::Programs::Basic> basicShadersProgram;
		std::unique_ptr<Shaders::Programs::Textured> texturedShadersProgram;

		struct
		{
			std::vector<Buffers::PosTexCoordBuffers> simpleBackgroundDecorations;
			std::vector<Buffers::PosTexCoordBuffers> simpleMidgroundDecorations;
			std::vector<Buffers::PosTexCoordBuffers> simpleForegroundDecorations;

			std::vector<Buffers::PosTexCoordBuffers> texturedBackgroundDecorations;
			std::vector<Buffers::PosTexCoordBuffers> texturedMidgroundDecorations;
			std::vector<Buffers::PosTexCoordBuffers> texturedForegroundDecorations;

			std::vector<Buffers::PosTexCoordBuffers> customShadersBackgroundDecorations;
			std::vector<Buffers::PosTexCoordBuffers> customShadersMidgroundDecorations;
			std::vector<Buffers::PosTexCoordBuffers> customShadersForegroundDecorations;
		} persistentBuffers;

		struct
		{
			std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers> simpleBackgroundDecorations;
			std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers> simpleMidgroundDecorations;
			std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers> simpleForegroundDecorations;

			std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers> texturedBackgroundDecorations;
			std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers> texturedMidgroundDecorations;
			std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers> texturedForegroundDecorations;

			std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers> customShadersBackgroundDecorations;
			std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers> customShadersMidgroundDecorations;
			std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers> customShadersForegroundDecorations;
		} temporaryBuffers;
	};
}
