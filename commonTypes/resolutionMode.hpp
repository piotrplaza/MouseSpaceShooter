#pragma once

enum class ResolutionMode {
	Normal,
	NormalLinearBlend0,
	NormalLinearBlend1,
	LowerLinearBlend0,
	LowerLinearBlend1,
	LowestLinearBlend0,
	LowestLinearBlend1,
	PixelArtBlend0,
	PixelArtBlend1,
	LowPixelArtBlend0,
	LowPixelArtBlend1
};

namespace ShadersUtils::Programs
{
	struct Textured;
}

class TexturesFramebuffersRenderer
{
public:
	TexturesFramebuffersRenderer(ShadersUtils::Programs::Textured& texturedShadersProgram);
	~TexturesFramebuffersRenderer();

	void clearIfFirstOfMode(ResolutionMode resolutionMode);

private:
	ShadersUtils::Programs::Textured& texturedShadersProgram;

	bool normalLinearBlend0 = false;
	bool normalLinearBlend1 = false;
	bool lowerLinearBlend0 = false;
	bool lowerLinearBlend1 = false;
	bool lowestLinearBlend0 = false;
	bool lowestLinearBlend1 = false;
	bool pixelArtBlend0 = false;
	bool pixelArtBlend1 = false;
	bool lowPixelArtBlend0 = false;
	bool lowPixelArtBlend1 = false;
};
