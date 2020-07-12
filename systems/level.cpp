#include "level.hpp"

#include <GL/glew.h>

#include <Box2D/Box2D.h>

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>

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

		simpleStaticWallsBuffers = std::make_unique<PosTexCoordBuffers>();
		simpleDynamicWallsBuffers = std::make_unique<PosTexCoordBuffers>();
		simpleGrapplesBuffers = std::make_unique<PosTexCoordBuffers>();

		updateStaticWallsPositionsBuffers();
		updateGrapplesPositionsBuffers();
		updateTexCoordsBuffers();
	}

	void Level::updateStaticWallsPositionsBuffers()
	{
		Tools::UpdateSimpleAndTexturesPositionsBuffers(Globals::Components::staticWalls,
			*simpleStaticWallsBuffers, texturesToStaticWallsBuffers, customSimpleStaticWallsBuffers,
			customTexturedStaticWallsBuffers, GL_STATIC_DRAW);
	}

	void Level::updateDynamicWallsPositionsBuffers()
	{
		Tools::UpdateSimpleAndTexturesPositionsBuffers(Globals::Components::dynamicWalls,
			*simpleDynamicWallsBuffers, texturesToDynamicWallsBuffers, customSimpleDynamicWallsBuffers,
			customTexturedDynamicWallsBuffers, GL_DYNAMIC_DRAW);
	}

	void Level::updateGrapplesPositionsBuffers()
	{
		Tools::UpdateSimpleAndTexturesPositionsBuffers(Globals::Components::grapples,
			*simpleGrapplesBuffers, texturesToGrapplesBuffers, customSimpleGrapplesBuffers,
			customTexturedGrapplesBuffers, GL_STATIC_DRAW);
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
	}

	void Level::render() const
	{
		sceneCoordTexturedRender();
		texturedRender();
		basicRender();
	}

	void Level::sceneCoordTexturedRender() const
	{
		using namespace Globals::Components;

		glUseProgram_proxy(sceneCoordTexturedShadersProgram->program);
		sceneCoordTexturedShadersProgram->mvpUniform.setValue(mvp.getVP());

		for (const auto& [texture, texturedStaticWallBuffers] : texturesToStaticWallsBuffers)
		{
			const auto& textureComponent = textures[texture];
			const auto& textureDefComponent = texturesDef[texture];

			sceneCoordTexturedShadersProgram->texture1Uniform.setValue(texture);
			sceneCoordTexturedShadersProgram->textureTranslateUniform.setValue(textureDefComponent.translate);
			sceneCoordTexturedShadersProgram->textureScaleUniform.setValue(
				{ (float)textureComponent.height / textureComponent.width * textureDefComponent.scale.x, textureDefComponent.scale.y });
			glBindVertexArray(texturedStaticWallBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, texturedStaticWallBuffers.positionsCache.size());
		}
	}

	void Level::texturedRender() const
	{
		using namespace Globals::Components;

		glUseProgram_proxy(texturedShadersProgram->program);
		texturedShadersProgram->mvpUniform.setValue(mvp.getVP());

		for (const auto& [texture, texturedDynamicWallBuffers] : texturesToDynamicWallsBuffers)
		{
			const auto& textureComponent = textures[texture];
			const auto& textureDefComponent = texturesDef[texture];

			texturedShadersProgram->texture1Uniform.setValue(texture);
			texturedShadersProgram->textureTranslateUniform.setValue(textureDefComponent.translate);
			texturedShadersProgram->textureScaleUniform.setValue(
				{ (float)textureComponent.height / textureComponent.width * textureDefComponent.scale.x, textureDefComponent.scale.y });
			glBindVertexArray(texturedDynamicWallBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, texturedDynamicWallBuffers.positionsCache.size());
		}

		for (const auto& [texture, texturedGrappleBuffers] : texturesToGrapplesBuffers)
		{
			const auto& textureComponent = textures[texture];
			const auto& textureDefComponent = texturesDef[texture];

			texturedShadersProgram->texture1Uniform.setValue(texture);
			texturedShadersProgram->textureTranslateUniform.setValue(textureDefComponent.translate);
			texturedShadersProgram->textureScaleUniform.setValue(
				{ (float)textureComponent.height / textureComponent.width * textureDefComponent.scale.x, textureDefComponent.scale.y });
			glBindVertexArray(texturedGrappleBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, texturedGrappleBuffers.positionsCache.size());
		}
	}

	void Level::basicRender() const
	{
		using namespace Globals::Components;

		glUseProgram_proxy(basicShadersProgram->program);
		basicShadersProgram->mvpUniform.setValue(mvp.getVP());

		basicShadersProgram->colorUniform.setValue({ 0.5f, 0.5f, 0.5f, 1.0f });
		glBindVertexArray(simpleStaticWallsBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleStaticWallsBuffers->positionsCache.size());

		for (const auto& customSimpleStaticWallBuffers : customSimpleStaticWallsBuffers)
		{
			customSimpleStaticWallBuffers.renderingSetup(basicShadersProgram->program);
			glBindVertexArray(customSimpleStaticWallBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, customSimpleStaticWallBuffers.positionsCache.size());
		}

		basicShadersProgram->colorUniform.setValue({ 0.5f, 0.5f, 0.5f, 1.0f });
		glBindVertexArray(simpleDynamicWallsBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleDynamicWallsBuffers->positionsCache.size());

		basicShadersProgram->colorUniform.setValue({ 0.0f, 0.5f, 0.0f, 1.0f });
		glBindVertexArray(simpleGrapplesBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleGrapplesBuffers->positionsCache.size());
	}

	Level::PosTexCoordBuffers::PosTexCoordBuffers()
	{
		glCreateVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &positionBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
	}

	Level::PosTexCoordBuffers::~PosTexCoordBuffers()
	{
		glDeleteBuffers(1, &positionBuffer);
		if (texCoordBuffer) glDeleteBuffers(1, &*texCoordBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}

	void Level::PosTexCoordBuffers::createTexCoordBuffer()
	{
		assert(!texCoordBuffer);
		texCoordBuffer = 0;
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &*texCoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, *texCoordBuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(1);
	}
}
