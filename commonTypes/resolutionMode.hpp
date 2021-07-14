#pragma once

enum class ResolutionMode { Normal, LowerLinear, LowestLinear, PixelArt };

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

	bool lowerLinear = false;
	bool lowestLinear = false;
	bool pixelArt = false;
};
