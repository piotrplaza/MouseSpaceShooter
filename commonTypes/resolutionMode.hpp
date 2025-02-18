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
		Nearest,
		Linear,
		COUNT
	} scaling;

	enum class Blending {
		Standard,
		Additive,
		COUNT
	} blending;

	bool isDefaultMode() const
	{
		return resolution == defaultResolution && scaling == defaultScaling && blending == defaultBlending;
	}

	static constexpr Resolution defaultResolution = Resolution::Native;
	static constexpr Scaling defaultScaling = Scaling::Nearest;
	static constexpr Blending defaultBlending = Blending::Standard;
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
