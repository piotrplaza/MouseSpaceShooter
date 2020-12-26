#include <ogl/shaders.hpp>
#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/buffers/posTexCoordBuffers.hpp>

namespace Systems
{
	class Variables
	{
	public:
		Variables();

		void step();
		void render() const;

	private:
		void initGraphics();

		void updatePositionsBuffers();
		void updateTexCoordsBuffers();

		void customShadersRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const;
		void texturedRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const;
		void basicRender(const std::vector<Buffers::PosTexCoordBuffers>& buffer) const;

		std::unique_ptr<Shaders::Programs::Basic> basicShadersProgram;
		std::unique_ptr<Shaders::Programs::Textured> texturedShadersProgram;

		std::vector<Buffers::PosTexCoordBuffers> simpleRocketsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> texturedRocketsBuffers;
		std::vector<Buffers::PosTexCoordBuffers> customShaderRocketsBuffers;
	};
}
