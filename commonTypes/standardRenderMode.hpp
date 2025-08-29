#pragma once

#include <commonTypes/componentMappers.hpp>

#include <ogl/oglProxy.hpp>

#include <glm/glm.hpp>

#include <unordered_set>

struct StandardRenderMode
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
	};

	enum class Scaling {
		Nearest,
		Linear,
		COUNT
	};

	enum class Blending {
		Standard,
		Additive,
		COUNT
	};

	StandardRenderMode()
		: resolution(mainResolution), scaling(mainScaling), blending(mainBlending)
	{
	}

	StandardRenderMode(Resolution resolution, Scaling scaling, Blending blending)
		: resolution(resolution), scaling(scaling), blending(blending)
	{
	}

	StandardRenderMode(Resolution resolution)
		: resolution(resolution), scaling(mainScaling), blending(mainBlending)
	{
	}

	StandardRenderMode(Scaling scaling)
		: resolution(mainResolution), scaling(scaling), blending(mainBlending)
	{
	}

	StandardRenderMode(Blending blending)
		: resolution(mainResolution), scaling(mainScaling), blending(blending)
	{
	}

	bool isMainMode() const
	{
		return resolution == StandardRenderMode::mainResolution && scaling == StandardRenderMode::mainScaling && blending == StandardRenderMode::mainBlending;
	}

	static constexpr Resolution mainResolution = Resolution::Native;
	static constexpr Scaling mainScaling = Scaling::Nearest;
	static constexpr Blending mainBlending = Blending::Standard;

	Resolution resolution = mainResolution;
	Scaling scaling = mainScaling;
	Blending blending = mainBlending;
};

namespace ShadersUtils::Programs
{
	struct Textured;
}

class StandardRenderTexturesRenderer
{
public:
	StandardRenderTexturesRenderer(ShadersUtils::Programs::Textured& texturedShadersProgram);
	~StandardRenderTexturesRenderer();

	void clearIfFirstOfRenderTexture(const CM::RenderTexture& renderTexture);

private:
	ShadersUtils::Programs::Textured& texturedShadersProgram;
	bool ongoingModes[(size_t)StandardRenderMode::Resolution::COUNT][(size_t)StandardRenderMode::Scaling::COUNT][(size_t)StandardRenderMode::Blending::COUNT]{};
};

class CustomRenderTexturesRenderer
{
public:
	void clearIfFirstOfRenderTexture(const CM::RenderTexture& renderTexture);

private:
	std::unordered_set<CM::RenderTexture> ongoingRenderTextures;
};
