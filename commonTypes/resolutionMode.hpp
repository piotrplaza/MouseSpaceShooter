#pragma once

struct ResolutionMode
{
	enum class Resolution {
		Native,
		HalfNative,
		QuarterNative,
		OctaNative,
		H2160,
		H1080,
		H540,
		H405,
		H270,
		H135,
		H68,
		H34,
		H17,
		COUNT
	} resolution;

	enum class Scaling {
		Linear,
		Nearest,
		COUNT
	} scaling;

	enum class Blending {
		Standard,
		Additive,
		COUNT
	} blending;

	bool isMainMode() const
	{
		return resolution == Resolution::Native && scaling == Scaling::Linear && blending == Blending::Standard;
	}
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

	void clearIfFirstOfMode(ResolutionMode mode);

private:
	ShadersUtils::Programs::Textured& texturedShadersProgram;
	bool ongoingModes[(size_t)ResolutionMode::Resolution::COUNT][(size_t)ResolutionMode::Scaling::COUNT][(size_t)ResolutionMode::Blending::COUNT]{};
};
