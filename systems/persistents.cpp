#include "Persistents.hpp"

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
#include <components/graphicsSettings.hpp>

namespace Systems
{
	Persistents::Persistents()
	{
		initGraphics();
	}

	void Persistents::initGraphics()
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

	void Persistents::updateStaticWallsPositionsBuffers()
	{
		Tools::UpdateTransformedPositionsBuffers(Globals::Components::staticWalls,
			*simpleStaticWallsBuffers, texturesToStaticWallsBuffers, customSimpleStaticWallsBuffers,
			customTexturedStaticWallsBuffers, customShadersStaticWallsBuffers, GL_STATIC_DRAW);
	}

	void Persistents::updateDynamicWallsPositionsBuffers()
	{
		Tools::UpdateTransformedPositionsBuffers(Globals::Components::dynamicWalls,
			*simpleDynamicWallsBuffers, texturesToDynamicWallsBuffers, customSimpleDynamicWallsBuffers,
			customTexturedDynamicWallsBuffers, customShadersDynamicWallsBuffers, GL_DYNAMIC_DRAW);
	}

	void Persistents::updateGrapplesPositionsBuffers()
	{
		Tools::UpdateTransformedPositionsBuffers(Globals::Components::grapples,
			*simpleGrapplesBuffers, texturesToGrapplesBuffers, customSimpleGrapplesBuffers,
			customTexturedGrapplesBuffers, customShadersGrapplesBuffers, GL_DYNAMIC_DRAW);
	}

	void Persistents::updateTexCoordsBuffers()
	{
		updateDynamicWallsTexCoordsBuffers();
		updateGrapplesTexCoordsBuffers();
	}

	void Persistents::updateDynamicWallsTexCoordsBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components::dynamicWalls, texturesToDynamicWallsBuffers,
			customTexturedDynamicWallsBuffers, customShadersDynamicWallsBuffers, GL_STATIC_DRAW);
	}

	void Persistents::updateGrapplesTexCoordsBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components::grapples, texturesToGrapplesBuffers,
			customTexturedGrapplesBuffers, customShadersGrapplesBuffers, GL_STATIC_DRAW);
	}

	void Persistents::step()
	{
		updateDynamicWallsPositionsBuffers();
		updateGrapplesPositionsBuffers();
	}

	void Persistents::render() const
	{
		customShadersRender();
		sceneCoordTexturedRender();
		texturedRender();
		basicRender();
	}

	void Persistents::customShadersRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const
	{
		for (const auto& currentBuffers : buffers)
		{
			glUseProgram_proxy(*currentBuffers.customShadersProgram);

			std::function<void()> renderingTeardown;
			if (currentBuffers.renderingSetup)
				renderingTeardown = currentBuffers.renderingSetup(*currentBuffers.customShadersProgram);

			glBindVertexArray(currentBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, currentBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}
	}

	void Persistents::customShadersRender() const
	{
		customShadersRender(customShadersDynamicWallsBuffers);
		customShadersRender(customShadersStaticWallsBuffers);
		customShadersRender(customShadersGrapplesBuffers);
	}

	void Persistents::sceneCoordTexturedRender() const
	{
		glUseProgram_proxy(sceneCoordTexturedShadersProgram->program);
		sceneCoordTexturedShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());

		for (const auto& [texture, texturedStaticWallBuffers] : texturesToStaticWallsBuffers)
		{
			sceneCoordTexturedShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			sceneCoordTexturedShadersProgram->modelUniform.setValue(glm::mat4(1.0f));
			Tools::TexturedRender(*sceneCoordTexturedShadersProgram, texturedStaticWallBuffers, texture);
		}

		for (const auto& customTexturedStaticWallBuffers : customTexturedStaticWallsBuffers)
		{
			sceneCoordTexturedShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			sceneCoordTexturedShadersProgram->modelUniform.setValue(glm::mat4(1.0f));
			Tools::TexturedRender(*sceneCoordTexturedShadersProgram, customTexturedStaticWallBuffers,
				*customTexturedStaticWallBuffers.texture);
		}
	}

	void Persistents::texturedRender() const
	{
		glUseProgram_proxy(texturedShadersProgram->program);
		texturedShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());

		for (const auto& [texture, texturedDynamicWallBuffers] : texturesToDynamicWallsBuffers)
		{
			texturedShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			texturedShadersProgram->modelUniform.setValue(glm::mat4(1.0f));
			Tools::TexturedRender(*texturedShadersProgram, texturedDynamicWallBuffers, texture);
		}

		for (const auto& [texture, texturedGrappleBuffers] : texturesToGrapplesBuffers)
		{
			texturedShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			texturedShadersProgram->modelUniform.setValue(glm::mat4(1.0f));
			Tools::TexturedRender(*texturedShadersProgram, texturedGrappleBuffers, texture);
		}

		for (const auto& customTexturedDynamicWallBuffers : customTexturedDynamicWallsBuffers)
		{
			texturedShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			texturedShadersProgram->modelUniform.setValue(glm::mat4(1.0f));
			Tools::TexturedRender(*texturedShadersProgram, customTexturedDynamicWallBuffers,
				*customTexturedDynamicWallBuffers.texture);
		}

		for (const auto& customTextureGrappleBuffers : customTexturedGrapplesBuffers)
		{
			texturedShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			texturedShadersProgram->modelUniform.setValue(glm::mat4(1.0f));
			Tools::TexturedRender(*texturedShadersProgram, customTextureGrappleBuffers,
				*customTextureGrappleBuffers.texture);
		}
	}

	void Persistents::basicRender() const
	{
		glUseProgram_proxy(basicShadersProgram->program);
		basicShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());
		basicShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
		basicShadersProgram->modelUniform.setValue(glm::mat4(1.0f));

		glBindVertexArray(simpleStaticWallsBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleStaticWallsBuffers->positionsCache.size());

		glBindVertexArray(simpleDynamicWallsBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleDynamicWallsBuffers->positionsCache.size());

		glBindVertexArray(simpleGrapplesBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleGrapplesBuffers->positionsCache.size());

		for (const auto& customSimpleStaticWallBuffers : customSimpleStaticWallsBuffers)
		{
			basicShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			basicShadersProgram->modelUniform.setValue(glm::mat4(1.0f));

			std::function<void()> renderingTeardown =
				customSimpleStaticWallBuffers.renderingSetup(basicShadersProgram->program);

			glBindVertexArray(customSimpleStaticWallBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, customSimpleStaticWallBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}

		for (const auto& customSimpleDynamicWallBuffers : customSimpleDynamicWallsBuffers)
		{
			basicShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			basicShadersProgram->modelUniform.setValue(glm::mat4(1.0f));

			std::function<void()> renderingTeardown =
				customSimpleDynamicWallBuffers.renderingSetup(basicShadersProgram->program);

			glBindVertexArray(customSimpleDynamicWallBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, customSimpleDynamicWallBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}

		for (const auto& customSimpleGrappleBuffers : customSimpleGrapplesBuffers)
		{
			basicShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			basicShadersProgram->modelUniform.setValue(glm::mat4(1.0f));

			std::function<void()> renderingTeardown =
				customSimpleGrappleBuffers.renderingSetup(basicShadersProgram->program);

			glBindVertexArray(customSimpleGrappleBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, customSimpleGrappleBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}
	}
}
