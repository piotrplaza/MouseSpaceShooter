#pragma once

#include "_typeComponentMappers.hpp"

#include "decoration.hpp"

#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/renderLayer.hpp>

#include <ogl/shaders.hpp>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <vector>
#include <optional>

struct Renderable
{
	Renderable(TextureComponentVariant texture = std::monostate{},
		std::optional<ComponentId> renderingSetup = std::nullopt,
		RenderLayer renderLayer = RenderLayer::Midground,
		std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
		texture(texture),
		renderingSetup(renderingSetup),
		renderLayer(renderLayer),
		customShadersProgram(customShadersProgram)
	{
	}

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec4> colors;
	std::vector<glm::vec2> texCoord;

	TextureComponentVariant texture;
	std::optional<ComponentId> renderingSetup;
	std::optional<Shaders::ProgramId> customShadersProgram;

	ResolutionMode resolutionMode = ResolutionMode::Normal;
	RenderLayer renderLayer = RenderLayer::Midground;

	GLenum drawMode = GL_TRIANGLES;
	GLenum bufferDataUsage = GL_STATIC_DRAW;

	bool preserveTextureRatio = false;

	bool render = true;

	std::vector<DecorationDef> subsequence;
	unsigned posInSubsequence = 0;
};
