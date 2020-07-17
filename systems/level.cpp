#include "level.hpp"

#include <GL/glew.h>

#include <Box2D/Box2D.h>

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>
#include <ogl/renderingHelpers.hpp>

#include <globals.hpp>

#include <components/physics.hpp>
#include <components/mvp.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/texture.hpp>
#include <components/textureDef.hpp>

namespace Systems
{
	Level::Level()
	{
		initGraphics();
	}

	void Level::initGraphics()
	{
		basicShadersProgram = std::make_unique<Shaders::Programs::Basic>();
		sceneCoordTexturedShadersProgram = std::make_unique<Shaders::Programs::SceneCoordTextured>();
		texturedShadersProgram = std::make_unique<Shaders::Programs::Textured>();

		simpleStaticWallsBuffers = std::make_unique<Buffers::PosTexCoordBuffers>();
		simpleDynamicWallsBuffers = std::make_unique<Buffers::PosTexCoordBuffers>();
		simpleGrapplesBuffers = std::make_unique<Buffers::PosTexCoordBuffers>();

		updateStaticWallsPositionsBuffers();
		updateTexCoordsBuffers();
	}

	void Level::updateStaticWallsPositionsBuffers()
	{
		Tools::UpdateTransformedPositionsBuffers(Globals::Components::staticWalls,
			*simpleStaticWallsBuffers, texturesToStaticWallsBuffers, customSimpleStaticWallsBuffers,
			customTexturedStaticWallsBuffers, GL_STATIC_DRAW);
	}

	void Level::updateDynamicWallsPositionsBuffers()
	{
		Tools::UpdateTransformedPositionsBuffers(Globals::Components::dynamicWalls,
			*simpleDynamicWallsBuffers, texturesToDynamicWallsBuffers, customSimpleDynamicWallsBuffers,
			customTexturedDynamicWallsBuffers, GL_DYNAMIC_DRAW);
	}

	void Level::updateGrapplesPositionsBuffers()
	{
		Tools::UpdateTransformedPositionsBuffers(Globals::Components::grapples,
			*simpleGrapplesBuffers, texturesToGrapplesBuffers, customSimpleGrapplesBuffers,
			customTexturedGrapplesBuffers, GL_DYNAMIC_DRAW);
	}

	void Level::updateTexCoordsBuffers()
	{
		updateDynamicWallsTexCoordsBuffers();
		updateGrapplesTexCoordsBuffers();
	}

	void Level::updateDynamicWallsTexCoordsBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components::dynamicWalls, texturesToDynamicWallsBuffers,
			customTexturedDynamicWallsBuffers, GL_STATIC_DRAW);
	}

	void Level::updateGrapplesTexCoordsBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components::grapples, texturesToGrapplesBuffers,
			customTexturedGrapplesBuffers, GL_STATIC_DRAW);
	}

	void Level::step()
	{
		updateDynamicWallsPositionsBuffers();
		updateGrapplesPositionsBuffers();
	}

	void Level::render() const
	{
		sceneCoordTexturedRender();
		texturedRender();
		basicRender();
	}

	void Level::sceneCoordTexturedRender() const
	{
		glUseProgram_proxy(sceneCoordTexturedShadersProgram->program);
		sceneCoordTexturedShadersProgram->mvpUniform.setValue(Globals::Components::mvp.getVP());
		sceneCoordTexturedShadersProgram->colorUniform.setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

		for (const auto& [texture, texturedStaticWallBuffers] : texturesToStaticWallsBuffers)
			Tools::TexturedRender(*sceneCoordTexturedShadersProgram, texturedStaticWallBuffers, texture);

		for (const auto& customTexturedStaticWallBuffers : customTexturedStaticWallsBuffers)
			Tools::TexturedRender(*sceneCoordTexturedShadersProgram, customTexturedStaticWallBuffers,
				*customTexturedStaticWallBuffers.texture);
	}

	void Level::texturedRender() const
	{
		glUseProgram_proxy(texturedShadersProgram->program);
		texturedShadersProgram->mvpUniform.setValue(Globals::Components::mvp.getVP());
		texturedShadersProgram->colorUniform.setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

		for (const auto& [texture, texturedDynamicWallBuffers] : texturesToDynamicWallsBuffers)
			Tools::TexturedRender(*texturedShadersProgram, texturedDynamicWallBuffers, texture);

		for (const auto& [texture, texturedGrappleBuffers] : texturesToGrapplesBuffers)
			Tools::TexturedRender(*texturedShadersProgram, texturedGrappleBuffers, texture);

		for (const auto& customTexturedDynamicWallBuffers : customTexturedDynamicWallsBuffers)
			Tools::TexturedRender(*texturedShadersProgram, customTexturedDynamicWallBuffers,
				*customTexturedDynamicWallBuffers.texture);

		for (const auto& customTextureGrappleBuffers : customTexturedGrapplesBuffers)
			Tools::TexturedRender(*texturedShadersProgram, customTextureGrappleBuffers,
				*customTextureGrappleBuffers.texture);
	}

	void Level::basicRender() const
	{
		glUseProgram_proxy(basicShadersProgram->program);
		basicShadersProgram->mvpUniform.setValue(Globals::Components::mvp.getVP());
		basicShadersProgram->colorUniform.setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

		glBindVertexArray(simpleStaticWallsBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleStaticWallsBuffers->positionsCache.size());

		glBindVertexArray(simpleDynamicWallsBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleDynamicWallsBuffers->positionsCache.size());

		glBindVertexArray(simpleGrapplesBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleGrapplesBuffers->positionsCache.size());

		for (const auto& customSimpleStaticWallBuffers : customSimpleStaticWallsBuffers)
		{
			customSimpleStaticWallBuffers.renderingSetup(basicShadersProgram->program);
			glBindVertexArray(customSimpleStaticWallBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, customSimpleStaticWallBuffers.positionsCache.size());
		}

		for (const auto& customSimpleGrappleBuffers : customSimpleGrapplesBuffers)
		{
			customSimpleGrappleBuffers.renderingSetup(basicShadersProgram->program);
			glBindVertexArray(customSimpleGrappleBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, customSimpleGrappleBuffers.positionsCache.size());
		}
	}
}
