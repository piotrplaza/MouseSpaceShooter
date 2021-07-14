#pragma once

enum class ResolutionMode { Normal, LowerLinearBlend0, LowerLinearBlend1, LowestLinearBlend0, LowestLinearBlend1, PixelArtBlend0, PixelArtBlend1, LowPixelArtBlend0, LowPixelArtBlend1};

namespace Shaders::Programs
{
	struct Textured;
}

class TexturesFramebuffersRenderer
{
public:
	TexturesFramebuffersRenderer(Shaders::Programs::Textured& texturedShadersProgram);
	~TexturesFramebuffersRenderer();

	void clearIfFirstOfMode(ResolutionMode resolutionMode);

private:
	Shaders::Programs::Textured& texturedShadersProgram;

	bool lowerLinearBlend0 = false;
	bool lowerLinearBlend1 = false;
	bool lowestLinearBlend0 = false;
	bool lowestLinearBlend1 = false;
	bool pixelArtBlend0 = false;
	bool pixelArtBlend1 = false;
	bool lowPixelArtBlend0 = false;
	bool lowPixelArtBlend1 = false;
};
